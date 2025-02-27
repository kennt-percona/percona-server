.. _apt_repo:

====================================================
Installing |Percona Server| on *Debian* and *Ubuntu*
====================================================

Ready-to-use packages are available from the |Percona Server| software repositories and the `download page <http://www.percona.com/downloads/Percona-Server-5.6/>`_.

Supported Releases:

* Debian:

 * 6.0 (squeeze)
 * 7.0 (wheezy)
 * 8.0 (jessie)

* Ubuntu:

 * 12.04LTS (precise)
 * 14.04LTS (trusty)
 * 15.04 (vivid)
 * 15.10 (wily)

Supported Platforms:

 * x86
 * x86_64 (also known as ``amd64``)

What's in each DEB package?
===========================

The ``percona-server-server-5.6`` package contains the database server itself, the ``mysqld`` binary and associated files.

The ``percona-server-common-5.6`` package contains files common to the server and client.

The ``percona-server-client-5.6`` package contains the command line client.

The ``percona-server-5.6-dbg`` package contains debug symbols for the server.

The ``percona-server-test-5.6`` package contains the database test suite.

The ``percona-server-source-5.6`` package contains the server source.

The ``libperconaserverclient18.1-dev`` package contains header files needed to compile software to use the client library.

The ``libperconaserverclient18.1`` package contains the client shared library. The ``18.1`` is a reference to the version of the shared library. The version is incremented when there is a ABI change that requires software using the client library to be recompiled or its source code modified.
                   
Installing |Percona Server| from Percona ``apt`` repository
===========================================================

1. Import the public key for the package management system

  *Debian* and *Ubuntu* packages from *Percona* are signed with the Percona's GPG key. Before using the repository, you should add the key to :program:`apt`. To do that, run the following commands as root or with sudo:

  .. code-block:: bash

    $ sudo apt-key adv --keyserver keys.gnupg.net --recv-keys 1C4CBDCDCD2EFD2A

  .. note::

    In case you're getting timeouts when using ``keys.gnupg.net`` as an alternative you can fetch the key from ``keyserver.ubuntu.com``. 

2. Create the :program:`apt` source list for Percona's repository:

   You can create the source list and add the percona repository by running: 

   .. code-block:: bash

     $ echo "deb http://repo.percona.com/apt "$(lsb_release -sc)" main" | sudo tee /etc/apt/sources.list.d/percona.list

   Additionally you can enable the source package repository by running: 

   .. code-block:: bash 

     $ echo "deb-src http://repo.percona.com/apt "$(lsb_release -sc)" main" | sudo tee -a /etc/apt/sources.list.d/percona.list

3. Remember to update the local cache:

   .. code-block:: bash

     $ sudo apt-get update

4. After that you can install the server package:

   .. code-block:: bash

     $ sudo apt-get install percona-server-server-5.6 


Percona ``apt`` Testing repository
----------------------------------

Percona offers pre-release builds from the testing repository. To enable it add the just add the ``testing`` word at the end of the percona repository definition in your repository file (default :file:`/etc/apt/sources.list.d/percona.list`). It should looks like this (in this example ``VERSION`` is the name of your distribution): :: 

  deb http://repo.percona.com/apt VERSION main testing
  deb-src http://repo.percona.com/apt VERSION main testing

Apt-Pinning the packages
------------------------

In some cases you might need to "pin" the selected packages to avoid the upgrades from the distribution repositories. You'll need to make a new file :file:`/etc/apt/preferences.d/00percona.pref` and add the following lines in it: :: 

  Package: *
  Pin: release o=Percona Development Team
  Pin-Priority: 1001

For more information about the pinning you can check the official `debian wiki <http://wiki.debian.org/AptPreferences>`_.

.. _standalone_deb:

Installing |Percona Server| using downloaded deb packages
=========================================================

Download the packages of the desired series for your architecture from the `download page <http://www.percona.com/downloads/Percona-Server-5.6/>`_. The easiest way is to download bundle which contains all the packages. Following example will download |Percona Server| 5.6.25-73.1 release packages for *Debian* 8.0:  

 .. code-block:: bash

   $ wget https://www.percona.com/downloads/Percona-Server-5.6/Percona-Server-5.6.25-73.1/binary/debian/jessie/x86_64/Percona-Server-5.6.25-73.1-r07b797f-jessie-x86_64-bundle.tar 

You should then unpack the bundle to get the packages:

 .. code-block:: bash

   $ tar xvf Percona-Server-5.6.25-73.1-r07b797f-jessie-x86_64-bundle.tar

After you unpack the bundle you should see the following packages:

  .. code-block:: bash

    $ ls *.deb
    libperconaserverclient18.1-dev_5.6.25-73.1-1.jessie_amd64.deb
    libperconaserverclient18.1_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-5.6-dbg_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-client-5.6_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-client_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-common-5.6_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-server-5.6_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-server_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-source-5.6_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-test-5.6_5.6.25-73.1-1.jessie_amd64.deb
    percona-server-tokudb-5.6_5.6.25-73.1-1.jessie_amd64.deb

Now you can install |Percona Server| by running:

  .. code-block:: bash 

    $ sudo dpkg -i *.deb

This will install all the packages from the bundle. Another option is to download/specify only the packages you need for running |Percona Server| installation (``libperconaserverclient18.1_5.6.25-73.1-1.jessie_amd64.deb``, ``percona-server-client-5.6_5.6.25-73.1-1.jessie_amd64.deb``, ``percona-server-common-5.6_5.6.25-73.1-1.jessie_amd64.deb``, and ``percona-server-server-5.6_5.6.25-73.1-1.jessie_amd64.deb``). 

.. note:: 

  When installing packages manually like this, you'll need to make sure to resolve all the dependencies and install missing packages yourself.


Running |Percona Server|
========================

|Percona Server| stores the data files in :file:`/var/lib/mysql/` by default. You can find the configuration file that is used to manage |Percona Server| in :file:`/etc/mysql/my.cnf`. *Debian* and *Ubuntu* installation automatically creates a special ``debian-sys-maint`` user which is used by the control scripts to control the |Percona Server| ``mysqld`` and ``mysqld_safe`` services. Login details for that user can be found in :file:`/etc/mysql/debian.cnf`. 

1. Starting the service

   |Percona Server| is started automatically after it gets installed unless it encounters errors during the installation process. You can also manually start it by running: 

   .. code-block:: bash

     $ sudo service mysql start

2. Confirming that service is running 

   You can check the service status by running:  

   .. code-block:: bash

     $ service mysql status

3. Stopping the service

   You can stop the service by running:

   .. code-block:: bash

     $ sudo service mysql stop

4. Restarting the service 

   You can restart the service by running: 

   .. code-block:: bash

     $ sudo service mysql restart

.. note:: 

  *Debian* 8.0 (jessie) and *Ubuntu* 15.04 (vivid) come with `systemd <http://freedesktop.org/wiki/Software/systemd/>`_ as the default system and service manager so you can invoke all the above commands with ``sytemctl`` instead of ``service``. Currently both are supported.
     
Uninstalling |Percona Server|
=============================

To uninstall |Percona Server| you'll need to remove all the installed packages. Removing packages with :command:`apt-get remove` will leave the configuration and data files. Removing the packages with :command:`apt-get purge` will remove all the packages with configuration files and data files (all the databases). Depending on your needs you can choose which command better suits you.

1. Stop the |Percona Server| service

   .. code-block:: bash

     $ sudo service mysql stop 

2. Remove the packages
   
   a) Remove the packages. This will leave the data files (databases, tables, logs, configuration, etc.) behind. In case you don't need them you'll need to remove them manually.

   .. code-block:: bash

     $ sudo apt-get remove percona-server*

   b) Purge the packages. **NOTE**: This will remove all the packages and delete all the data files (databases, tables, logs, etc.)

   .. code-block:: bash

     $ sudo apt-get purge percona-server*


