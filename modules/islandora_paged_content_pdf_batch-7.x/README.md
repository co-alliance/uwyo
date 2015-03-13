# Islandora Paged Content PDF Batch [![Build Status](https://travis-ci.org/discoverygarden/islandora_paged_content_pdf_batch.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_paged_content_pdf_batch)

## Introduction

This module extends the [Islandora Batch](https://github.com/islandora/islandora_batch) framework to facilitate the ingestion of
a ZIP filled with one or more PDFs into paged content and individual page objects.

The ingest is a two-step process:

* Preprocessing: The data is scanned and a number of entries are created in the
  Drupal database.  There is minimal processing done at this point, so preprocessing can
  be completed outside of a batch process.
* Ingest: The data is actually processed and ingested. This happens inside of
  a Drupal batch.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Paged Content](https://github.com/islandora/islandora_paged_content)
* [Islandora Batch](https://github.com/islandora/islandora_batch)


# Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

### Usage

The base ZIP preprocessor can be called as a drush script (see `drush help islandora_paged_content_pdf_batch_preprocess` for additional parameters):

`drush -v -u 1 --uri=http://localhost islandora_paged_content_pdf_batch_preprocess --target=/path/to/archive.zip --content-model=islandora:bookCModel --parent=islandora:bookCollection`

This will populate the queue (stored in the Drupal database) with base entries.

The queue of preprocessed items can then be processed:

`drush -v -u 1 --uri=http://localhost islandora_batch_ingest`

Currently, the ingester has only been tested with the [Book](https://github.com/islandora/islandora_solution_pack_book) and [Newspaper](https://github.com/islandora/islandora_solution_pack_newspaper) solution packs. Other paged content may need to extend and customize the batch as noted below.

### Customization

Custom ingests can be written by [extending](https://github.com/Islandora/islandora_batch/wiki/How-To-Extend) any of the existing preprocessors and batch object implementations.

## Troubleshooting/Issues

Having problems or solved a problem? Contact [discoverygarden](http://support.discoverygarden.ca).

## Maintainers/Sponsors

This project has been sponsored by:

* [discoverygarden](http://wwww.discoverygarden.ca)

## Development

If you would like to contribute to this module, please check out our helpful
[Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers)
info, [Developers](http://islandora.ca/developers) section on Islandora.ca and
contact [discoverygarden](http://support.discoverygarden.ca).

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
