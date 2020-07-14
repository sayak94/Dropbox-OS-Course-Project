*SERVER SIDE*

1. First edit the ip address in the ip.conf file. Put the ip address of the server.
2. Open terminal in the Server named Folder.
   Compile code: ./compile_server.sh
   Run code: ./server /pathname/foldername/
     For eg: ./server /home/baban/Desktop/sayak/
	 "sayak" is the name of the folder that will act as our dropbox and must be created before running the code.
	 All its contents will be synchronised across all clients that connect to the server at a time.
3. Open the folder(like "sayak" in our example) and see how changes in the client side gets updated in the server side.

*CLIENT SIDE*

1. First edit the ip address in the ip.conf file. Put the ip address of the server.
2. Open terminal in the Client named Folder.
   Compile code: ./compile_client.sh
   Run code: ./client
3. A prompt will ask for the folder path to be synchronised. Enter pathname/foldername/.
   For eg: /home/deepsan/Desktop/deep/
   "deep" is the name of the folder to be synchronised with the server.
   if "deep" was already present then a prompt will ask whether you want to keep the folder or replace. If "deep" is absent a new folder will be created.
   Upon replace or upon creation of "deep" all contents from server side dropbox("sayak" folder's contents in our example) will be sent to "deep".
4. Open the "deep" folder. Do all kind of operations like file/folder creation, file/folder deletion, copy pasting, cut pasting, Moving files and folders containing subfolders and files, etc.
   Check in the server side folder("sayak" in our case) all changes in the client side will be replicated.
5. Upon completion of all work in the client side. Close the client terminal or press Ctrl+C to exit the Dropbox from the client side.
6. Now a new client is ready to connect to the server and all changes made by the previous client will be reflected as soon as he connects to the server.
   Follow the same steps from step 1 to connect a new client. Do not close the server.
7. Note: We have assumed that a person will not use Dropbox at home and office at the same time and should also exit the terminal where he previously
   made changes(for example in office)so as to connect to the server from new client location(for example at home).
   

   
