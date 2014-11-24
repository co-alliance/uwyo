CONTENTS OF THIS FILE
---------------------

 * summary
 * requirements
 * installation
 * configuration
 * customization
 * troubleshooting
 * faq
 * contact
 * sponsors


SUMMARY
-------

PALS Batch Newspaper Ingester

Extends the functionality in islandora_book_batch to accomodate ingesting issues of newspapers.

Each issue has a metadata file called "MODS.xml", which must include a "dateIssued" field. The image files must be
named "OBJ.tiff". The directory structure used is the same as that for the islandora book batch module.



REQUIREMENTS
------------

Dependent on:
* islandora_batch
* islandora_book_batch
* islandora_large_image

INSTALLATION
------------

Install as any other Drupal module.

CONFIGURATION
-------------

There are a couple relevant parameters...  They are available from the drush
help output for the command. Primarily:
* target: Used to indicate the directory in which to search for the ingest files
* parent: Used to indicate what newspaper the newspaper issue objects should
  be made a member of.


CUSTOMIZATION
-------------


TROUBLESHOOTING
---------------


F.A.Q.
------


CONTACT
-------


SPONSORS
--------

