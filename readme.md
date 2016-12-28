# setuppvc
## Automated iscsi to openshift 3.x persistent volume creation
This automatically looks at iscsi targets found by the host, and creates
the appropriate yml file, and adds the iscsi device as a openshift Persistent volume. 

It will keep the yml files, and only add new persistent volumes.

Tested with Dell Compellent Arrays. Use the Dell Storage manager to create
as many luns as needed, then run setupppvc to add them to openshift.

In theory should work with any iscsi device.

Note, this has not been tested where u have boot devices mixed with luns for
openshift, and most likely will cause disaster. 



