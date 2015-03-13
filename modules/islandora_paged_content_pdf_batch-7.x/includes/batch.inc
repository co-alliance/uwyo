<?php

/**
 * @file
 * Implementation of classes from the batch ingest framework for books.
 */

/**
 * Batch preprocessor.
 */
class IslandoraPagedContentPDFBatch extends IslandoraScanBatch {
  /**
   * Static function to get object class.
   */
  protected static function getObjectClass() {
    return 'IslandoraPagedContentPDFBatchParentObject';
  }

  /**
   * Get the type of the target resource.
   */
  protected function getTargetType() {
    return 'zip';
  }

  /**
   * Preprocess all the things.
   */
  public function preprocess() {
    $this->parameters['set_id'] = $this->getSetId();
    $added = parent::preprocess();

    $object_type = static::getObjectClass();

    foreach ($added as $object) {
      if ($object instanceof $object_type) {
        $children = db_select('islandora_batch_queue', 'q')
          ->fields('q', array('id'))
          ->condition('parent', $object->id);

        // Set pages ready to go.
        db_update('islandora_batch_state')
          ->fields(array(
            'state' => ISLANDORA_BATCH_STATE__READY,
          ))
          ->condition('id', $children, 'IN')
          ->execute();
        // Set book ready to go (after children, that is).
        db_update('islandora_batch_state')
          ->fields(array(
            'state' => ISLANDORA_BATCH_STATE__PENDING_CHILDREN,
          ))
          ->condition('id', $object->id)
          ->execute();
      }
    }

    return $added;
  }
}

/**
 * Constructs the parent object for the paged content.
 */
class IslandoraPagedContentPDFBatchParentObject extends IslandoraScanBatchObject {
  /**
   * Constructor for the IslandoraScanBatchObject.
   */
  public function __construct(IslandoraTuque $connection, $base_name, $object_info, $preprocessor_parameters) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    parent::__construct($connection, $base_name, $object_info, $preprocessor_parameters);
    $zip_file = $this->objectInfo['pdf'];
    $scheme = file_default_scheme();
    $pdf_file = file_create_filename($zip_file->filename, "$scheme://");
    copy($zip_file->uri, $pdf_file);
    $file_obj = islandora_temp_file_entry($pdf_file);
    file_usage_add($file_obj, 'islandora_batch', 'islandora_batch_set', $preprocessor_parameters['set_id']);
    $this->objectInfo['pdf_file'] = $file_obj->fid;
  }

  /**
   * Returns the label as the title for the object.
   */
  public function getTitle() {
    return $this->label;
  }

  /**
   * Get the children of the paged content parent.
   */
  public function getChildren(IslandoraTuque $connection) {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    // Create page objects, return in an array.
    $children = array();
    $file = file_load($this->objectInfo['pdf_file']);
    $length = islandora_paged_content_length_of_pdf($file->uri);
    if (!$length) {
      throw new Exception(t('Unable to extract length of PDF.'));
    }
    $pages = range(1, $length);
    foreach ($pages as $sequence) {
      $children[] = new IslandoraPagedContentPDFBatchPageObject($connection, $this->id, $this->preprocessorParameters['content_model'], $sequence, $file->fid, $this->preprocessorParameters);
    }
    return $children;
  }

  /**
   * Add the content model relationship(s).
   */
  protected function addContentModelRelationships() {
    $this->models = $this->preprocessorParameters['content_model'];
  }

  /**
   * Function batch_process.
   */
  public function batchProcess() {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    $file = file_load($this->objectInfo['pdf_file']);
    islandora_paged_content_add_pdf_to_object($this, $file, TRUE);
    // Use object_info to create some datastreams.
    $this->label = $file->filename;
    return ISLANDORA_BATCH_STATE__DONE;
  }
}

/**
 * Constructs the child objects for paged content.
 */
class IslandoraPagedContentPDFBatchPageObject extends IslandoraScanBatchObject {
  protected $parentId;
  protected $sequenceNumber;
  protected $textExtraction;
  protected $language;
  protected $displayFormat;
  protected $resolution;
  protected $pdfFile;

  /**
   * Constructorsaurusrex.
   */
  public function __construct(IslandoraTuque $connection, $parent_id, $parent_cmodel, $sequence, $fid, $pp) {
    $this->pdfFile = $fid;
    $file = file_load($fid);
    // Change the label of the page to the sequence number if variable is set.
    if (variable_get('islandora_paged_content_page_label', FALSE)) {
      $label = $sequence;
    }
    else {
      $label = "$file->filename-$sequence";
    }
    parent::__construct($connection, NULL, array(), $pp);
    $this->label = $label;
    $this->parentId = $parent_id;
    $this->sequenceNumber = $sequence;
    $this->textExtraction = $pp['text'];
    $this->language = $pp['language'];
    $this->displayFormat = $pp['format'];
    $this->resolution = $pp['resolution'];
    $this->parentCModel = $parent_cmodel;
  }

  /**
   * Returns the label as the title for the object.
   */
  public function getTitle() {
    return $this->label;
  }

  /**
   * Batch process that goes out and constructs the datastreams for the child.
   */
  public function batchProcess() {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    module_load_include('inc', 'islandora', 'includes/utilities');
    $file = file_load($this->pdfFile);
    $pdf = islandora_paged_content_chop_pdf($file->uri, $this->sequenceNumber);
    if ($pdf) {
      islandora_paged_content_update_datastream($this, $pdf, 'PDF', 'PDF', 'application/pdf', 'M');
      $pdf_file = islandora_temp_file_entry($pdf);
      file_usage_add($pdf_file, 'islandora_batch', 'islandora_batch_object', $this->getBatchId());
    }
    else {
      watchdog('islandora_paged_content_pdf_batch', 'Unable to chop a PDF from the PDF for @pid.', array('@pid' => $this->id));
      return ISLANDORA_BATCH_STATE__ERROR;
    }
    $tif = islandora_paged_content_extract_tiff_from_pdf($file->uri, $this->sequenceNumber, $this->displayFormat, $this->resolution);
    if ($tif) {
      islandora_paged_content_update_datastream($this, $tif, 'OBJ', 'OBJ Datastream', 'image/tiff', 'M', FALSE);
      $tif_file = islandora_temp_file_entry($tif);
      file_usage_add($tif_file, 'islandora_batch', 'islandora_batch_object', $this->getBatchId());
    }
    else {
      watchdog('islandora_paged_content_pdf_batch', 'Unable to extract a TIF from the PDF for @pid.', array('@pid' => $this->id));
      return ISLANDORA_BATCH_STATE__ERROR;
    }
    if ($this->textExtraction == 'extract') {
      $full_text = islandora_paged_content_extract_text_from_pdf($file->uri, $this->sequenceNumber);
      if ($full_text) {
        islandora_paged_content_update_datastream($this, $full_text, 'OCR', 'OCR', 'text/plain', 'M', FALSE);
        $full_text_file = islandora_temp_file_entry($full_text);
        file_usage_add($full_text_file, 'islandora_batch', 'islandora_batch_object', $this->getBatchId());
      }
      else {
        watchdog('islandora_paged_content_pdf_batch', 'Unable to extract text from the PDF for @pid.', array('@pid' => $this->id));
        return ISLANDORA_BATCH_STATE__ERROR;
      }
    }
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Returns the children which there are none.
   */
  public function getChildren(IslandoraTuque $connection) {
    // Pages should not have children.
    return array();
  }

  /**
   * Adds relationships needed for the children of paged content.
   */
  public function addRelationships() {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    // Add relationship to collection.
    $rels_ext = $this->relationships;
    islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $this->parentId);
    islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $this->sequenceNumber, TRUE);
    islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $this->sequenceNumber, TRUE);
    islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
    islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $this->parentId);

    if ($this->textExtraction == "none") {
      $generate_ocr = 'FALSE';
    }
    else {
      $generate_ocr = 'TRUE';
    }
    $this->relationships->add(ISLANDORA_RELS_EXT_URI, 'generate_ocr', $generate_ocr, RELS_TYPE_PLAIN_LITERAL);
    if ($generate_ocr == 'TRUE') {
      islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'hasLanguage', $this->language, TRUE);
    }
    // Add content model relationship.
    $paged_cmodels = islandora_paged_content_retrieve_applicable_cmodels();
    $this->models = array_keys($paged_cmodels[$this->parentCModel]['children']);
  }
}
