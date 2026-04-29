# CS469_FINAL_GROUP_PROJECT
A collaborative space for completion of the final group project.

For reference, our project specification is listed below:

Our team is going to build a file server that will export a portion of the local file system to a client. With this, it will have a failover to a concurrent file server running on the same machine (with a different port). This file server is therefore responsible for centralized and networked access to storage, and allows for file access from multiple ports. 
Distribution transparency will be maintained through concealing the physical location of the files, replication of the local file data, and migration of said data across multiple ports. All of this is invisible to the user, but allows for effective access to the files. Additionally, in the event of an error, it will automatically switch to the failover file server without any detection from the user.
 
The use of cryptographic methods for authentication will increase the security of this project. Through the use of SSL and SSH, we will implement key-based authentication and digital certificates.  

Replication will be achieved through the data, as the goal of this project is to replicate a file from the system and send it over to the client. The goal with this replication is to have consistent and reliable data passed to the client, which will in part be achieved by the use of TCP sockets. On an application level, reading the file information in chunks, as well as having the backup failover file server will ensure reliability and consistency. 
Fault tolerance is achieved by detecting a primary server failure, and switching automatically to the backup server. Through this, the system is able to gracefully handle faults while also maintaining transparency for the user.


** TO RUN THE CODE** (in your terminal)
1. Create a test file for yourself within the exported_dir folder. We generally just make a .txt file with assorted information to test.
2. run make
3. run ./server 4433 (or desired port)
4. run ./server 4434 (or desired failover port) <-- creates failover server
5. run ./client localhost
6. You will be prompted to attach a file. Use the test file that you created in step 1.
7. If you wish to test the failover properties, simply delete the terminal running the primary server. This simulates a severed connection, and the client will automatically connect with the failover server.

