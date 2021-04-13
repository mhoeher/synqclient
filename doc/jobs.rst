Jobs
====

In order to talk to a remote server, the library implements a job based system. All jobs ultimately derive from the :any:`SynqClient::AbstractJob` class. There are several classes which directly derive from this class. These classes model the available functions the library provides to talk with a server, like up- and downloading files, listing a remote directory and so on.


Abstract Job Interface
++++++++++++++++++++++

These jobs are all *abstract* in the sense that they only describe a generic interface.


AbstractJob
-----------

.. doxygenclass:: SynqClient::AbstractJob
    :members:


GetFileInfoJob
--------------

.. doxygenclass:: SynqClient::GetFileInfoJob
    :members:


ListFilesJob
------------

.. doxygenclass:: SynqClient::ListFilesJob
    :members:


CreateDirectoryJob
------------------

.. doxygenclass:: SynqClient::CreateDirectoryJob
    :members:


UploadFileJob
-------------

.. doxygenclass:: SynqClient::UploadFileJob
    :members:


DownloadFileJob
---------------

.. doxygenclass:: SynqClient::DownloadFileJob
    :members:


DeleteJob
---------

.. doxygenclass:: SynqClient::DeleteJob
    :members:


Additional Type Definitions and Functions
-----------------------------------------


JobError
........

.. doxygenenum:: SynqClient::JobError


JobState
........

.. doxygenenum:: SynqClient::JobState


FileInfo
........

.. doxygenclass:: SynqClient::FileInfo


FileInfos
.........

.. doxygentypedef:: SynqClient::FileInfos


Concrete Job Classes
++++++++++++++++++++


In addition to these abstract classes, there are concrete classes that implement concrete access to a server of a particular type. The following server types are currently supported:


.. toctree::
   :maxdepth: 1

   webdav
   dropbox


Higher Level Job Utilities
++++++++++++++++++++++++++

The abstract core jobs describe low level (quasi atomic) operations against a remote service. Usually, concrete applications will built on top of them to make use of their functionality, creating higher order functions. To avoid repetitive work in applications using SynqClient, the library also provides some higher order functionality to work with jobs.


CompositeJob
------------

The :any:`SynqClient::CompositeJob` class is a job class (i.e. it derives from the :any:`SynqClient::AbstractJob` class) and hence, implements the same job interface like any other job. The purpose of this class is to allow combining several other jobs together into one larger batch of jobs, that can be run at once.

.. doxygenclass:: SynqClient::CompositeJob


Additional Type Definitions
---------------------------

The following type definitions are used together with the higher level job interface:


CompositeJobErrorMode
.....................

.. doxygenenum:: SynqClient::CompositeJobErrorMode
