<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function pals_newspaper_batch_drush_command() {
  $items = array();

  $items['pals_newspaper_batch_preprocess'] = array(
    'aliases' => array('ibbp'),
    'description' => 'Preprocesses newspaper issues from folder structure into database entries.',
    'drupal dependencies' => array('pals_newspaper_batch', 'islandora_book', 'islandora_batch',
      'islandora_book_batch', 'islandora_newspaper'),
    'options' => array(
      'type' => array(
        'description' => 'Either "directory" or "zip".',
        'required' => TRUE,
      ),
      'target' => array(
        'description' => 'The target to directory to scan.',
        'required' => TRUE,
      ),
      'namespace' => array(
        'description' => 'The namespace into which to ingest.',
        'required' => 'FALSE',
      ),
      'content_models' => array(
        'description' => 'A comma-separated list of content models to assign to the objects. Only applies to the newspaper issue level object.',
        'value' => 'optional',
      ),
      'parent' => array(
        'description' => 'The collection to which the generated items should be added.  Only applies to the "newspaper issue" level object.',
        'value' => 'optional',
      ),
      'parent_relationship_uri' => array(
        'description' => 'The namespace URI of the relationship to the parent. Defaults to "info:fedora/fedora-system:def/relations-external#".',
        'value' => 'optional',
      ),
      'parent_relationship_pred' => array(
        'description' => 'The predicate of the relationship to the parent. Defaults to "isMemberOf".',
        'value' => 'optional',
      ),
      'create_pdfs' => array(
        'description' => 'A flag to cause PDFs to be created in books. Page PDF creation is dependant on the configuration within Drupal proper.',
        'value' => 'optional',
      ),
      'do_not_generate_ocr' => array(
        'description' => 'A flag to allow for conditional OCR generation.',
        'value' => 'optional',
      ),
      'email_admin' => array(
        'description' => 'A flag to notify the site admin when the book is ' .
        'fully ingested (depends on Rules being enabled).',
        'value' => 'optional',
      ),
      'wait_for_metadata' => array(
        'description' => 'A flag to indicate that we should hold off on ' .
        'trying to ingest books until we have metadata available for them' .
        'at the book level.',
        'value' => 'optional',
      ),
      'directory_dedup' => array(
        'description' => 'A flag to indicate that we should avoid ' .
        'repreprocessing books which are located in directories.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  return $items;
}

/**
 * Drush command callback; perform the newspaper preprocessing.
 */
function drush_pals_newspaper_batch_preprocess() {
  // Build one of our preprocessors.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  $parameters = array(
    'type' => drush_get_option('type'),
    'namespace' => drush_get_option('namespace'),
    'target' => drush_get_option('target'),
    'parent' => drush_get_option('parent', 'islandora:newspaperCollection'),
    'parent_relationship_uri' => drush_get_option('parent_relationship_uri', 'info:fedora/fedora-system:def/relations-external#'),
    'parent_relationship_pred' => drush_get_option('parent_relationship_pred', 'isMemberOf'),
    'create_pdfs' => drush_get_option('create_pdfs', FALSE),
    'email_admin' => drush_get_option('email_admin', FALSE),
    'wait_for_metadata' => drush_get_option('wait_for_metadata', FALSE),
    'directory_dedup' => drush_get_option('directory_dedup', FALSE),
  );

  if ($content_models = drush_get_option('content_models', FALSE)) {
    $parameters['content_models'] = explode(',', $content_models);
  }
  else {
    $parameters['content_models'] = array('islandora:newspaperIssueCModel');
  }

  $preprocessor = new PALSNewspaperBatchPreprocessor($connection, $parameters);

  // Pass the preprocessor off to run.
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
}
