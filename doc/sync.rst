Synchronization
===============

The main goal of SynqClient is to provide means to synchronize a local and a remote folder. Such a folder may contain arbitrary files and also sub-folders. However, it is assumed that the structure to be synchronized is *well-defined* and does not change too much over time. In particular, the sync procedure does not cover the following:

- Moves and copies of files or entire folders are not detected. Hence, if they occur, the library will simply upload/download the duplicate/moved data.
- Changing a path from being a file to a folder or vice versa is not supported.

This is basically by design: The use case for the synchronization as implemented in SynqClient is to allow a program to store its state in a well defined folder structure and keep it in sync between devices. Such a programatically created structure usually has a quite fixed design. For example, the OpenTodoList app, form which this library has been factored out, used the following approach:

- The app organized various items - notes, images and todo lists - in *libraries*.
- Each such library is represented as a folder consisting of sub-folders and files.
- When a new item is created, it gets a path assigned using the following approach: `<library_root_folder>/<year>/<month>/<item_file_name>.txt`.
  - In other words, inside the library folder, we have in the first level below one folder per year in which an item has been created. Inside each year's folder we have one folder per month. Finally, inside the per-month folder, we put the actual files.

As you can see, this plays well with the *limitations* of the sync procedure: Items never get moved around. Additionally, there will never be a case where a path is converted from a file to a folder or vice versa.

If you can design your app to store its information in a similar manner, than SynqClient should work just fine for you to keep the app state in sync between devices!

The rest of this chapter will list all the ingredients we need to sync a local and a remote folder.


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

.. doxygenclass:: SynqClient::DropboxJobFactory
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

Log messages produced by the synchronizer use the :any:`SynqClient::SynchronizerLogEntryType` enumeration to encode the concrete type of log message.

.. doxygenenum:: SynqClient::SynchronizerLogEntryType