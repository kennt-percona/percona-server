/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
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

#ifndef _TOKUDB_BACKGROUND_H
#define _TOKUDB_BACKGROUND_H

#include "hatoku_hton.h"
#include <list>

namespace tokudb {
namespace background {

class job_manager_t {
public:
    class job_t {
    public:
        virtual ~job_t(void) {}

        // method that runs the job
        virtual void run(void) = 0;

        // method that tells the job to cancel ASAP
        virtual void cancel(void) = 0;

        // method that tells the job to clean up/free resources on cancel
        // or completion
        virtual void destroy(void) = 0;

        // method that tells the job to free thyself
        virtual void free(void) = 0;

        // method that returns a 'key' string for finding a specific job
        // (or jobs) usually used to find jobs to cancel
        virtual const char* key(void) = 0;

        // method to get info for information schema, 255 chars per buffer
        virtual void status(char* database, char* table, char* type,
                            char* params, char* status) = 0;

    private:
        friend class job_manager_t;
        bool        _running;
        bool        _cancelling;
        uint64_t    _id;
        bool        _user_scheduled;
        time_t      _scheduled_time;
        time_t      _started_time;
    };

    // pfn for iterate callback
    typedef void (*pfn_iterate_t)(uint64_t, const char*, const char*,
                                  const char*, const char*, const char*,
                                  bool, time_t, time_t, void*);

public:
    void* operator new(size_t sz);
    void operator delete(void* p);

    job_manager_t(void);

    ~job_manager_t(void);

    // creates/initializes a singleton bjm
    void initialize(void);

    // destroys a bjm singleton
    // cancels all jobs abd frees all resources
    void destroy(void);

    // schedules or runs a job depending on the 'background' value
    // job specifics all depend on the implementation od 'job'
    // background jobs will be executed in a FIFO fashion
    // two jobs with the same key can not run concurrently
    // if a foreground job is attempted, any currently scheduled or running
    // background jobs will be cancelled first
    // if another foreground job is already running, a new foreground job with
    // the same key will be rejected
    uint64_t run_job(job_t* newjob, bool user, bool background);

    // cancels any background job with a matching key
    bool cancel_job(const char* key);

    // iterates currently pending and running background jobs, calling
    // 'callback' with the 'extra' data provided and the original 'extra'
    // data passed when the job was scheduled
    void iterate_jobs(pfn_iterate_t callback, void* extra) const;

private:
    static void* thread_func(void* v);

    void* real_thread_func();

    // _mutex MUST be held on entry, will release and reaquire on exit
    void run(job_t* job);

    // _mutex MUST be held on entry
    void cancel(job_t* job);

private:
    typedef std::list<job_t*> jobs_t;

    mutable tokudb::thread::mutex_t     _mutex;
    mutable tokudb::thread::semaphore_t _sem;
    mutable tokudb::thread::thread_t    _thread;
    uint64_t        _next_id;
    jobs_t          _background_jobs;
    jobs_t          _foreground_jobs;
    bool            _shutdown;

};

extern job_manager_t*    _job_manager;

bool initialize(void);
bool destroy(void);
} // namespace background
} // namespace tokudb

#endif // _TOKUDB_BACKGROUND_H
