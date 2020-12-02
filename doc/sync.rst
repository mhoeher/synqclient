Synchronization
===============

The main task of SynqClient is to provide means to synchronize a local and a remote directory over arbitrary protocols. The synchronization itself is implemented on top of the job framework - meaning: It does not use specific properties of a dedicated protocol or server, but entirely relies on the information which is available via the various defined jobs.

For this reason, the synchronization code itself is written against the abstract job base classes, like :any:`SynqClient::GetFileInfoJob`, :any:`SynqClient::ListFilesJob` and :any:`SynqClient::DownloadFileJob`. In order to make these classes available to the sync code, a factory class is used.


AbstractJobFactory
++++++++++++++++++

The AbstractJobFactory class is the base class for concrete factories that are then used in conjunction with the code for the actual synchronization. It just defines the available protocol but on its own it does not provide any functionality.


.. doxygenclass:: SynqClient::AbstractJobFactory
    :members:


The :any:`SynqClient::JobType` enum is used by the protected class interface to determine which
concrete kind of job shall be created:

.. doxygenenum:: SynqClient::JobType



Concrete Factories
++++++++++++++++++

The following concrete factory classes are provided by SynqClient in order to allow synchronization against specific backend servers and protocols:

.. doxygenclass:: SynqClient::WebDAVJobFactory
    :members: