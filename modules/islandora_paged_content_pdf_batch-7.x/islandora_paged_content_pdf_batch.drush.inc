<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_paged_content_pdf_batch_drush_command() {
  $items = array();
  $items['islandora_paged_content_pdf_batch_preprocess'] = array(
    'aliases' => array('ipcpbp'),
    'description' => 'Preprocessed paged content and their children into database entries.',
    'drupal dependencies' => array('islandora_batch',
      'islandora_paged_content',
      'islandora_paged_content_pdf_batch'),
    'options' => array(
      'target' => array(
        'description' => dt('The path to the ZIP file to scan.'),
        'required' => TRUE,
      ),
      'text' => array(
        'description' => dt('One of the following options: "extract" to pull the text out of the PDF, "ocr" to run OCR against the PDF or "none". Defaults to "extract"'),
        'value' => 'optional',
      ),
      'language' => array(
        'description' => dt('Valid short code for OCR extraction. Defaults to "eng".'),
        'value' => 'optional',
      ),
      'format' => array(
        'description' => dt('The image format to use to when extracting images from PDF. Valid options are: @options. Defaults to "tiff32nc".', array(
          '@options' => implode(', ', array(
            'tiffgray',
            'tiff12nc',
            'tiff24nc',
            'tiff48nc',
            'tiff32nc',
            'tiff64nc',
          )),
        )),
        'value' => 'optional',
      ),
      'resolution' => array(
        'description' => dt('The resolution to use in pixels. Defaults to "300"'),
        'value' => 'optional',
      ),
      'content_model' => array(
        'description' => dt('The content model to assign to the parent objects.'),
        'required' => TRUE,
      ),
      'parent' => array(
        'description' => dt('The collection or pid to which the generated items should be added to.'),
        'required' => TRUE,
      ),
      'parent_relationship_uri' => array(
        'description' => dt('The namespace URI of the relationship to the parent. Defaults to "info:fedora/fedora-system:def/relations-external#".'),
        'value' => 'optional',
      ),
      'parent_relationship_pred' => array(
        'description' => dt('The predicate of the relationship to the parent. Defaults to "isMemberOfCollection".'),
        'value' => 'optional',
      ),
      'namespace' => array(
        'description' => dt('The namespace for objects created by this command.  Defaults to namespace set in Fedora config.'),
        'required' => FALSE,
      ),
    ),
    'examples' => array(
      'drush -v -u 1 --uri=http://localhost islandora_paged_content_pdf_batch_preprocess --target=/path/to/archive.zip --content_model=islandora:bookCModel --parent=islandora:bookCollection' => t('Preprocessing paged content.'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Batch preprocess validation handler.
 */
function drush_islandora_paged_content_pdf_batch_preprocess_validate() {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  // See if the target specified exists and is a ZIP.
  $target = drush_get_option('target');
  if (file_exists($target)) {
    $allowed_zips = array(
      'application/zip',
      'application/x-zip',
    );
    $mime = file_get_mimetype($target);
    if (!in_array($mime, $allowed_zips)) {
      return drush_set_error('Invalid MIME', dt('The specified file is not a ZIP.'));
    }
  }
  else {
    return drush_set_error('Missing file', dt('Unable to locate the ZIP file specified at @zip.', array('@zip' => $target)));
  }
  // See if pdfinfo is available.
  if (!islandora_paged_content_pdfinfo_availability()) {
    return drush_set_error('Missing pdfinfo', dt('Unable to ingest as pdfinfo is missing.'));
  }
  if ($text = drush_get_option('text', 'extract')) {
    $valid_text_extraction = array(
      'extract',
      'ocr',
      'none',
    );
    if (!in_array($text, $valid_text_extraction)) {
      return drush_set_error('Invalid text extraction', dt('The specified text extraction type, !text, is not valid. Options available are "extract", "ocr" or "none".', array(
        '!text' => $text,
      )));
    }
    if ($text == 'extract') {
      // See if pdftotext is available.
      if (!islandora_paged_content_pdftotext_availability()) {
        return drush_set_error('Missing pdftotext', dt('Unable to ingest as pdftotext is missing and the extract option is selected.'));
      }
    }
  }
  if ($language = drush_get_option('language', 'eng')) {
    module_load_include('inc', 'islandora_paged_content_pdf_batch', 'includes/utilities');
    $valid_languages = islandora_paged_content_pdf_batch_available_languages();
    if (!in_array($language, $valid_languages)) {
      return drush_set_error('Invalid language entered', dt('The specified language, !language, is not valid. Languages available are: @lang.', array(
        '!language' => $language,
        '@lang' => implode(', ', $valid_languages),
      )));
    }
  }

  if ($format = drush_get_option('format', 'tiff32nc')) {
    $valid_formats = array(
      'tiffgray',
      'tiff12nc',
      'tiff24nc',
      'tiff48nc',
      'tiff32nc',
      'tiff64nc',
    );
    if (!in_array($format, $valid_formats)) {
      return drush_set_error('Invalid format entered', dt('The specified format, !format, is not valid. Formats available are: @formats.', array(
        '!format' => $format,
        '@formats' => implode(', ', $valid_formats),
      )));
    }
  }

  if ($resolution = drush_get_option('resolution', '300')) {
    if (!is_numeric($resolution) && !preg_match('/\d+x\d+/', $resolution)) {
      return drush_set_error('Invalid resolution entered', dt('The specified resolution, !resolution, is not valid.', array(
        '!resolution' => $resolution,
      )));
    }
  }
  $content_model = drush_get_option('content_model');
  $applicable_cmodels = islandora_paged_content_retrieve_applicable_cmodels();
  if (!in_array($content_model, array_keys($applicable_cmodels))) {
    return drush_set_error('Invalid content model specified', dt('The specified content model, !cmodel, is not valid.', array(
      '!cmodel' => $content_model,
    )));
  }
}

/**
 * Implements hook_drush_command().
 */
function drush_islandora_paged_content_pdf_batch_preprocess() {
  // XXX: Due to how Drush bootstrapping works, the connection may be created
  // without credentials (when your site's front page is
  // 'islandora/object/some:object', for example). Resetting to ensure a new
  // connection gets created should fix it.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  $parameters = array(
    'target' => drush_get_option('target'),
    'content_model' => drush_get_option('content_model'),
    'parent' => drush_get_option('parent'),
    'parent_relationship_uri' => drush_get_option('parent_relationship_uri', FEDORA_RELS_EXT_URI),
    'parent_relationship_pred' => drush_get_option('parent_relationship_pred', 'isMemberOfCollection'),
    'namespace' => drush_get_option('namespace'),
  );

  if ($text = drush_get_option('text', 'extract')) {
    $parameters['text'] = $text;
  }
  else {
    $parameters['text'] = 'extract';
  }

  if ($language = drush_get_option('language', 'eng')) {
    $parameters['language'] = $language;
  }
  else {
    $parameters['language'] = 'eng';
  }

  if ($format = drush_get_option('format', 'tiff32nc')) {
    $parameters['format'] = $format;
  }
  else {
    $parameters['format'] = 'tiff32nc';
  }

  if ($resolution = drush_get_option('resolution', '300')) {
    $parameters['resolution'] = $resolution;
  }
  else {
    $parameters['format'] = '300';
  }
  $preprocessor = new IslandoraPagedContentPDFBatch($connection, $parameters);
  islandora_batch_handle_preprocessor($preprocessor);
}
