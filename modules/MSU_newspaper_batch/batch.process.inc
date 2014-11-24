<?php

/**
 * @file
 * Classes and functions related to the islandora_batch framework.
 */

require_once '/var/www/drupal/sites/all/modules/islandora/islandora.module';
require_once '/var/www/drupal/sites/all/modules/islandora_solution_pack_newspaper/includes/utilities.inc';
require_once '/var/www/drupal/sites/all/modules/islandora_book_batch/includes/islandora_book_batch.inc';
// require_once '/var/www/drupal/sites/all/modules/islandora/includes/dublin_core.inc';


class PALSNewspaperBatchPreprocessor extends IslandoraBookBatch {
  /**
   * Inherits.
   */
  protected static function getObjectClass() {
    return 'PALSNewspaperIssueBatchObject';
  }
}

class PALSNewspaperIssueBatchObject extends IslandoraBookBookBatchObject {
  /**
   * Inherits.
   */

  /**
   * Function getChildren.
   */
  public function getChildren(IslandoraTuque $connection) {
    // Create page objects, return in an array.
    $children = array();

    foreach ($this->objectInfo as $sequence => $info) {
      if ($sequence != '.') {
        $children[] = new PALSNewspaperPageBatchObject($connection, $this->id, $sequence, $info, $this->preprocessorParameters);
      }
    }

    return $children;
  }

  /**
   * Function addRelationships.
   */
  public function addRelationships() {
    parent::addRelationships();
//    module_load_include('inc', 'islandora_paged_content', 'includes/derivatives');
    module_load_include('inc', 'islandora_solution_pack_newspaper', 'includes/utilities');
//    module_load_include('inc', 'islandora', 'includes/dublin_core');


//    if (isset($this->preprocessorParameters['create_pdfs']) && $this->preprocessorParameters['create_pdfs']) {
//      islandora_paged_content_set_pdf_flag($this);
//    }
    // Identify the source of this object.
//    $this->relationships->add(ISLANDORA_RELS_EXT_URI, 'book-batched', 'true', RELS_TYPE_PLAIN_LITERAL);
//    if (isset($this->preprocessorParameters['email_admin']) && $this->preprocessorParameters['email_admin']) {
//      $this->relationships->add(ISLANDORA_RELS_EXT_URI, 'email-admin-when-ingested', 'true', RELS_TYPE_PLAIN_LITERAL);
//    }

    $parent_object = islandora_object_load($this->preprocessorParameters['parent']);
    $issues = islandora_newspaper_get_issues($parent_object);
    $num_issues = count($issues) + 1;
    $this->relationships->add(ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $num_issues, TRUE);
  }
}

/**
 * Class PALSNewspaperPageBatchObject.
 */
class PALSNewspaperPageBatchObject extends IslandoraBookBookBatchObject {
//  protected $parentId;
//  protected $sequenceNumber;

  /**
   * Function construct.
   */
  public function __construct(IslandoraTuque $connection, $parent_id, $sequence, $object_info, $pp) {

    $label = "$parent_id-$sequence";
    // Change the label of the page to the sequence number if variable is set.
    if (variable_get('islandora_paged_content_page_label', FALSE)) {
      $label = $sequence;
    }

    parent::__construct($connection, $label, $object_info, $pp);

    $this->parentId = $parent_id;
    $this->sequenceNumber = $sequence;
  }

  /**
   * Function getChildren.
   */
  public function getChildren(IslandoraTuque $connection) {
    // Pages should not have children.
    return array();
  }

  /**
   * Function initialBatchState.
   */
  public function initialBatchState() {
    // Not ready.
    return ISLANDORA_BATCH_STATE__NOT_READY;
  }

/**
   * Function addRelationships.
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
    // Add content model relationship.
    $this->models = 'islandora:newspaperPageCModel';
    if (isset($this->preprocessorParameters['generate_ocr'])) {
      if ($this->preprocessorParameters['generate_ocr'] == TRUE) {
        module_load_include('inc', 'islandora_ocr', 'includes/derivatives');
        islandora_ocr_set_generate_ocr_rels_ext_statement($this);
      }
    }
  }
}
