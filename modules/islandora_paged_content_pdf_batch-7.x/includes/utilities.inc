<?php
/**
 * @file
 * Utility functions for PDF Batch ingest.
 */

/**
 * Retrieve valid languages to pass off to our batch.
 *
 * @return array
 *   An array where the values are the valid languages options that can be used.
 */
function islandora_paged_content_pdf_batch_available_languages() {
  if (module_exists('islandora_ocr')) {
    module_load_include('inc', 'islandora_ocr', 'includes/utilities');

    $valid_languages = islandora_ocr_get_enabled_tesseract_languages();
    // Don't need the no OCR option.
    unset($valid_languages['no_ocr']);
    $valid_languages = array_keys($valid_languages);
  }
  else {
    $valid_languages = array(
      'eng',
      'fra',
      'deu-frak',
      'por',
      'spa',
      'hin',
      'jpn',
      'ita',
    );
  }
  return $valid_languages;
}
