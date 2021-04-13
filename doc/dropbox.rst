Dropbox
=======

The Dropbox classes implement the abstract job interface to allow accessing files and folders stored on Dropbox. Besides the interface defined by the :any:`SynqClient::AbstractJob` as well as the abstract job base classes, all Dropbox related jobs also implement the :any:`SynqClient::AbstractDropboxJob` interface. This interface defines the shared properties of all Dropbox related functionality.


General Notes
-------------

The Dropbox job classes map like this to the library:

- The `rev` property as provided by Dropbox is mapped to the sync attribute (see e.g. :any:`SynqClient::FileInfo`).
- No additional custom properties are used in :any:`SynqClient::FileInfo`.


Warnings
--------

Please consider the following hints when working with the Dropbox jobs:

- The Dropbox jobs use an OAuth2 bearer token to identify the user. The library itself does not provide any functionality to acquire such a token. For testing purposes, you can create a token in the Dropbox app console for your app. For productive use, you will want to acquire a token by running through the appropriate OAuth2 flow. Qt provides OAuth support via the *Qt Network Authorization* module.


AbstractDropboxJob
------------------

.. doxygenclass:: SynqClient::AbstractDropboxJob
    :members:


DropboxGetFileInfoJob
---------------------

.. doxygenclass:: SynqClient::DropboxGetFileInfoJob
    :members:


DropboxListFilesJob
-------------------

.. doxygenclass:: SynqClient::DropboxListFilesJob
    :members:


DropboxCreateDirectoryJob
-------------------------

.. doxygenclass:: SynqClient::DropboxCreateDirectoryJob
    :members:


DropboxUploadFileJob
--------------------

.. doxygenclass:: SynqClient::DropboxUploadFileJob
    :members:


DropboxDownloadFileJob
----------------------

.. doxygenclass:: SynqClient::DropboxDownloadFileJob
    :members:


DropboxDeleteJob
----------------

.. doxygenclass:: SynqClient::DropboxDeleteJob
    :members:

