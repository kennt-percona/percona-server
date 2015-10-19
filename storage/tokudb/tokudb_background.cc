/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
/* -*- mode: C; c-basic-offset: 4 -*- */
#ident "$Id$"
/*======
This file is part of TokuDB


Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved.

    TokuDBis is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2,
    as published by the Free Software Foundation.

    TokuDB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TokuDB.  If not, see <http://www.gnu.org/licenses/>.

======= */

#ident "Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved."

#include "tokudb_background.h"
#include "tokudb_sysvars.h"

namespace tokudb {
namespace background {


job_manager_t* _job_manager = NULL;

bool initialize(void) {
    assert_always(_job_manager == NULL);
    _job_manager = new job_manager_t;
    _job_manager->initialize();
    return true;
}
bool destroy(void) {
    _job_manager->destroy();
    delete _job_manager;
    _job_manager = NULL;
    return true;
}
void* job_manager_t::operator new(size_t sz) {
    return tokudb::memory::malloc(sz, MYF(MY_WME|MY_ZEROFILL|MY_FAE));
}
void job_manager_t::operator delete(void* p) {
    tokudb::memory::free(p);
}
job_manager_t::job_manager_t(void) :
    _sem(0, 65535) {
}
job_manager_t::~job_manager_t(void) {
}
void job_manager_t::initialize(void) {
    _next_id = 1;
    _shutdown = false;

    int r = _thread.start(thread_func, this);
    assert_always(r == 0);
}
void job_manager_t::destroy(void) {
    assert_always(!_shutdown);
    assert_always(_foreground_jobs.size() == 0);
    _mutex.lock();
    _shutdown = true;
    if (_background_jobs.size()) {
        cancel(_background_jobs.front());
    }
    _sem.set_interrupt();
    _mutex.unlock();
    void* result;
    int r = _thread.join(&result);
    assert_always(r == 0);
    while (_background_jobs.size()) {
        _mutex.lock();
        job_t* job = _background_jobs.front();
        cancel(job);
        _background_jobs.pop_front();
        job->free();
        _mutex.unlock();
    }
}
uint64_t job_manager_t::run_job(job_t* newjob, bool user, bool background) {
    uint64_t id = ~0;
    const char* jobkey = newjob->key();

    _mutex.lock();
    assert_always(!_shutdown);

    for (jobs_t::iterator it = _background_jobs.begin();
         it != _background_jobs.end(); it++) {
        job_t* job = *it;
        if (!job->_cancelling && strcmp(job->key(), jobkey) == 0) {
            // if this is a foreground job being run and
            // there is an existing background job of the same type
            // and it is not running yet, we can cancel the background job
            // and just run this one in the foreground, might have different
            // params, but that is up to the user to figure out.
            if (!background && !job->_running) {
                cancel(job);
            } else {
                // can't schedule or run another job on the same key
                goto cleanup;
            }
        }
    }
    for (jobs_t::iterator it = _foreground_jobs.begin();
         it != _foreground_jobs.end(); it++) {
        job_t* job = *it;
        if (strcmp(job->key(), jobkey) == 0) {
            // can't schedule or run another job on the same key
            // as an existing foreground job
            goto cleanup;
        }
    }

    newjob->_running = false;
    newjob->_cancelling = false;
    newjob->_id = id = _next_id++;
    newjob->_user_scheduled = user;
    newjob->_scheduled_time = ::time(0);
    newjob->_started_time = 0;

    if (background) {
        _background_jobs.push_back(newjob);
        _sem.signal();
    } else {
        _foreground_jobs.push_back(newjob);

        run(newjob);

        for (jobs_t::iterator it = _foreground_jobs.begin();
             it != _foreground_jobs.end(); it++) {
            job_t* job = *it;
            if (job == newjob) {
                _foreground_jobs.erase(it);
                job->free();
                break;
            }
        }
    }

cleanup:
    _mutex.unlock();
    return id;
}
bool job_manager_t::cancel_job(const char* key) {
    bool ret = false;
    _mutex.lock();

    for (jobs_t::iterator it = _background_jobs.begin();
         it != _background_jobs.end(); it++) {
        job_t* job = *it;

        if (!job->_cancelling &&
            strcmp(job->key(), key) == 0) {

            cancel(job);

            ret = true;
        }
    }

    _mutex.unlock();
    return ret;
}
void job_manager_t::iterate_jobs(pfn_iterate_t callback, void* extra) const {

    char database[256], table[256], type[256], params[256], status[256];

    _mutex.lock();

    for (jobs_t::const_iterator it = _background_jobs.begin();
         it != _background_jobs.end(); it++) {
        job_t* job = *it;
        if (!job->_cancelling) {
            database[0] = table[0] = type[0] = params[0] = status[0] = '\0';
            job->status(database, table, type, params, status);
            callback(job->_id, database, table, type, params, status,
                     job->_user_scheduled, job->_scheduled_time,
                     job->_started_time, extra);
        }
    }

    _mutex.unlock();
}
void* job_manager_t::thread_func(void* v) {
    return ((tokudb::background::job_manager_t*)v)->real_thread_func();
}
void* job_manager_t::real_thread_func() {
    while (_shutdown == false) {
        tokudb::thread::semaphore_t::E_WAIT res = _sem.wait();
        if (res == tokudb::thread::semaphore_t::E_INTERRUPTED ||
            _shutdown == true) {
                break;
        } else if (res == tokudb::thread::semaphore_t::E_SIGNALLED) {
#if TOKUDB_DEBUG
            if (TOKUDB_UNLIKELY(tokudb::sysvars::debug_pause_background_job_manager
                == true)) {
                _sem.signal();
                tokudb::time::sleep_microsec(250000);
                continue;
            }
#endif // TOKUDB_DEBUG

            _mutex.lock();
            assert_debug(_background_jobs.size() > 0);
            job_t* job = _background_jobs.front();
            run(job);
            _background_jobs.pop_front();
            _mutex.unlock();
            job->free();
        }
    }
    return NULL;
}
void job_manager_t::run(job_t* job) {
    assert_debug(_mutex.is_owned_by_me());
    if (!job->_cancelling) {
        job->_running = true;
        job->_started_time = ::time(0);
        _mutex.unlock();
        // do job
        job->run();
        // done job
        job->_running = false;
        _mutex.lock();
    }
    if (!job->_cancelling) {
        job->destroy();
    }
}
void job_manager_t::cancel(job_t* job) {
    assert_debug(_mutex.is_owned_by_me());
    job->_cancelling = true;
    if (job->_running)
        job->cancel();
    while (job->_running) tokudb::time::sleep_microsec(500000);
    job->destroy();
}

} // namespace background
} // namespace tokudb
