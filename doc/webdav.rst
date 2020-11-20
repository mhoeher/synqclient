WebDAV
======

The WebDAV classes implement the abstract job classes to allow accessing WebDAV servers. As all of these classes require - besides the minimal :any:`SynqClient::AbstractJob` interface - also common shared properties, a new interface :any:`SynqClient::AbstractWebDAVJob` is introduced which adds all of the shared, WebDAV-specific attributes to the WebDAV job classes.


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

