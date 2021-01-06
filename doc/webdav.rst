WebDAV
======

The WebDAV classes are implementing the abstract job classes to allow accessing WebDAV servers. As all of these classes require - besides the minimal :any:`SynqClient::AbstractJob` interface - also common shared properties, a new interface :any:`SynqClient::AbstractWebDAVJob` is introduced which adds all of the shared, WebDAV-specific attributes to the WebDAV job classes.


General Notes
-------------

The WebDAV job classes map like this to the library:

- HTTP `etags` are mapped to sync attributes (see e.g. :any:`SynqClient::FileInfo`).
- No additional custom properties are used in :any:`SynqClient::FileInfo`.


Warnings
--------

Please consider the following hints when working with the WebDAV jobs:

- When uploading to the same file in quick succession, all of the upload jobs might return the same `etag`/sync attribute. Hence, if you rely on proper sync attributes, make sure you add a small pause between uploads to the same file. Usually, a second should be sufficient. When using the :any:`SynqClient::DirectorySynchronizer`, this usually shouldn't be an issue (most real world use cases will have a data base which takes some time to process and hence you anyway have some delay in between uploads). However, keep this in mind in case you use the functionality e.g. in artificial environments (namely, unit testing).
- Deleting folders conditionally does not work. If you delete a remote folder, make sure you don't set a sync attribute.



AbstractWebDAVJob
-----------------

.. doxygenclass:: SynqClient::AbstractWebDAVJob
    :members:


WebDAVGetFileInfoJob
--------------------

.. doxygenclass:: SynqClient::WebDAVGetFileInfoJob
    :members:


WebDAVListFilesJob
------------------

.. doxygenclass:: SynqClient::WebDAVListFilesJob
    :members:


WebDAVCreateDirectoryJob
------------------------

.. doxygenclass:: SynqClient::WebDAVCreateDirectoryJob
    :members:


WebDAVUploadFileJob
-------------------

.. doxygenclass:: SynqClient::WebDAVUploadFileJob
    :members:


WebDAVDownloadFileJob
---------------------

.. doxygenclass:: SynqClient::WebDAVDownloadFileJob
    :members:


WebDAVDeleteJob
---------------

.. doxygenclass:: SynqClient::WebDAVDeleteJob
    :members:


WebDAV Specific Types And Functions
-----------------------------------


WebDAVServerType
++++++++++++++++


.. doxygenenum:: SynqClient::WebDAVServerType

