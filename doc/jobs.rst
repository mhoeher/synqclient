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


Additional Type Definitions and Functions
-----------------------------------------


JobError
........

.. doxygenenum:: SynqClient::JobError


JobState
........

.. doxygenenum:: SynqClient::JobState


ItemProperty
............

.. doxygenclass:: SynqClient::ItemProperty
    :members:


ItemType
............

.. doxygenclass:: SynqClient::ItemType
    :members:



Concrete Job Classes
++++++++++++++++++++


In addition to these abstract classes, there are concrete classes that implement concrete access to a server of a particular type. The following server types are currently supported:


.. toctree::
   :maxdepth: 1

   webdav