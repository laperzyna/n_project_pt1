# n_project_pt1

## Right Now
pre-probing is basically done. Config file is still being hard coded but we can figure that out a little later.
RN the hard-coded config file is transferring from the client to the server all good and we are closing the connection
successfully. There is a check in the server for incoming bytes which helps the server know when the client is done and has left.

## TODO
probing phase -> UDP packets 
    - rn I was working on the UDP packets being sent but am still in the middle of making it work correctly
post-probing phase -> detecting compression