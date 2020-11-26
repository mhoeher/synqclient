Utils
=====

The utils package contains additional functionality which is somehow related to what a user of SynqClient typically needs to do and which is not strictly about remote file access or synchronization. In particular, the followiung utilities are included:


NextCloudLoginFlow
++++++++++++++++++

When working with NextCloud, a user needs to authenticate, usually providing a username and a password. In this regard, NextCloud behaves like a normal WebDAV server. However, besides using the user's genuine password, NextCloud also allows to use *app specific passwords*. This means: The user can - for each app they want to be able to communicate with NextCloud on their behalf - create a specific password. The advantage is, that it is sufficient to revoke this (and only this) password if it is compromised (e.g. if the device on which the app is used has been lost or stolen).

On the other side, while possible, it is cumbersome for a user to manually generate app-specific passwords manually. To aid this, NextCloud implements a `Login Flow <https://docs.nextcloud.com/server/latest/developer_manual/client_apis/LoginFlow/index.html>`_. This flow allows an app to request the NextCloud server to create a new app-specific password for the user and return the generated password.

.. doxygenclass:: SynqClient::NextCloudLoginFlow
    :members: