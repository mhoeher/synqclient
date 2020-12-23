Synchronization
===============

The main goal of SynqClient is to provide means to synchronize a local and a remote folder. This chapter will list all the ingredients we need to get to this target.



Remote Access
+++++++++++++

The most important ingredient is access to the "remote" folder. For this, the library provides a job based interface which should allow to implement access against a wide range of servers and protocols. The base class for all jobs is :any:`SynqClient::AbstractJob`. From this, specific (but still abstract) job types like the :any:`SynqClient::GetFileInfoJob`, :any:`SynqClient::DownloadFileJob` or :any:`SynqClient::DeleteJob` are derived. To implement access to a concrete server or talk over a specific protocol, these abstract classes need to be implemented. These concrete implementations can either be part of the SynqClient library or can be part of the host application (or a higher level library) which uses SynqClient.


Factories
+++++++++

The actual sync functionality is written in a high level manner, meaning: On that level, no details about the underlying protocol to access remote files is used (or desired). Consequentially, this part is written entirely against the abstract job classes mentioned above. However, as the sync functionality needs to create jobs on the fly, *factories* are used. Factories are subclasses of the :any:`SynqClient::AbstractJobFactory` class.

AbstractJobFactory
------------------

The AbstractJobFactory class is the base class for concrete factories that are then used in conjunction with the code for the actual synchronization. It just defines the available protocol but on its own it does not provide any functionality.


.. doxygenclass:: SynqClient::AbstractJobFactory
    :members:


The :any:`SynqClient::JobType` enum is used by the protected class interface to determine which
concrete kind of job shall be created:

.. doxygenenum:: SynqClient::JobType



Concrete Factories
------------------

The following concrete factory classes are provided by SynqClient in order to allow synchronization against specific backend servers and protocols:

.. doxygenclass:: SynqClient::WebDAVJobFactory
    :members:



Synchronization State Database
++++++++++++++++++++++++++++++

The next very important ingredient for the sync is the *State Database*. In order to implement proper synchronization, we need to keep track of some information between sync runs. This information must be saved persistently. In order to be as flexible as possible, the functionality to interact with such a database is done via another interface: The :any:`SynqClient::SyncStateDatabase` class.


SyncStateDatabase
-----------------

.. doxygenclass:: SynqClient::SyncStateDatabase

The :any:`SynqClient::SyncStateEntry` class is used to store information about a single entry in the sync database:

.. doxygenclass:: SynqClient::SyncStateEntry


Concrete Databases
------------------

Often, the functionality of such a synchronization database won't be different between applications. For this reason, SynqClient comes with the following default implementations, which can be used out of the box instead of implementing own ones:

.. doxygenclass:: SynqClient::SQLSyncStateDatabase

.. doxygenclass:: SynqClient::JSONSyncStateDatabase


Directory Synchronizer
++++++++++++++++++++++

With the above mentioned ingredients, we have everything at hand required to implement synchronization of a local and a remote folder. The synchronization part is implemented as a dedicated class - :any:`SynqClient::DirectorySynchronizer` - which is configured appropriately with concrete instances of the functional blocks described above.

.. doxygenclass:: SynqClient::DirectorySynchronizer

The :any:`SynqClient::SynchronizerError` enumeration is used to encode the various errors that might occur during the sync.

.. doxygenenum:: SynqClient::SynchronizerError

The :any:`SynqClient::SynchronizerState` is used to represent the states a synchronizer runs through.

.. doxygenenum:: SynqClient::SynchronizerState

In case a sync conflict occurs, the :any:`SynqClient::SyncConflictStrategy` enum is used to determine how to proceed.

.. doxygenenum:: SynqClient::SyncConflictStrategy

Some aspects of the synchronization can be controlled by passing a :any:`SynqClient::SynchronizerFlags` value to the synchronizer:

.. doxygentypedef:: SynqClient::SynchronizerFlags

The valid flags are encoded in the :any:`SynqClient::SynchronizerFlag` enum:

.. doxygenenum:: SynqClient::SynchronizerFlag

Functions suitable for being used as filters to determine of a particular file or folder shall be included in the synchronization are defined by the :any:`SynqClient::DirectorySynchronizer::Filter` type:

.. doxygentypedef:: SynqClient::DirectorySynchronizer::Filter