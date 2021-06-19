Introduction
============

SynqClient is a C++ library based on the Qt framework. It is rooted in the `OpenTodoList <https://opentodolist.rpdev.net>`_ project, which implemented a simple synchronization mechanism with WebDAV. The idea behind: The app would write all of its data in a directory structure. The synchronization would then simply keep this folder and a remote copy of it in sync.

While there might be more sophisticated solutions for this at hand, it proved to be quite usable. However, the initial code had two flaws:

- It was integrated into the app's code and hence not reusable.
- And on top, it broke with Qt's event driven paradigm, forcing a sync procedure to run all the necessary steps one by one, not leveraging any of the possibilities to speed up e.g. up- and downloads by pipelining multiple requests.

I took the decision to break out that part of the app and factor it into its own, independent library to fix both of these. Doing so would allow me (and others) to reuse the sync part for other apps. And on top, I wanted to re-design the code such that it is playing well with Qt, faster and easier to maintain.

And it does not stop there: The initial code was focusing on WebDAV only. The goal of SynqClient is to support other backend servers as well. WebDAV was - and still is - a great choice, because it is an open protocol with several open source web applications available talking it - this allows privacy focused apps like OpenTodoList to provide a sync functionality without handing over data to third parties. However, for users who don't want to host their own server, a sync via existing, well known backends is possible as well. In a first step, Dropbox support was added, a service that quite a lot of users have access to. And other services can be added quite easily as well.