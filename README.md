# CS469_FINAL_GROUP_PROJECT
A collaborative space for completion of the final group project.

For reference, our project specification is listed below:

Our team is going to build a file server that will export a portion of the local file system to a client. With this, it will have a failover to a concurrent file server running on the same machine (with a different port). This file server is therefore responsible for centralized and networked access to storage, and allows for file access from multiple ports. 
Distribution transparency will be maintained through concealing the physical location of the files, replication of the local file data, and migration of said data across multiple ports. All of this is invisible to the user, but allows for effective access to the files. Additionally, in the event of an error, it will automatically switch to the failover file server without any detection from the user.
 
The use of cryptographic methods for authentication will increase the security of this project. Through the use of SSL and SSH, we will implement key-based authentication and digital certificates.  

Replication will be achieved through the data, as the goal of this project is to replicate a file from the system and send it over to the client. The goal with this replication is to have consistent and reliable data passed to the client, which will in part be achieved by the use of TCP sockets. On an application level, reading the file information in chunks, as well as having the backup failover file server will ensure reliability and consistency. 
Fault tolerance is achieved by detecting a primary server failure, and switching automatically to the backup server. Through this, the system is able to gracefully handle faults while also maintaining transparency for the user.

