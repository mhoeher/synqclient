.. SynqClient documentation master file, created by
   sphinx-quickstart on Sun Nov 15 17:19:40 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to SynqClient's documentation!
======================================

SynqClient is a C++ library based on Qt which implements job-based access to file sharing services. On top, it implements synchronization between a local folder structure and a remote one on a server. At the moment, the core library supports the following backends:

- WebDAV (this allows data access and synchronization against a wide range of services, including NextCloud and ownCloud).
- Dropbox.

Due to the job system, it is easy to add support for additional storage backends and re-use the synchronization mechanism by plugging these custom job classes into the framework provided by the library.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   introduction
   api


Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
