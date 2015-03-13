<?php
/**
 * @file
 * Holds the UI upload portion for the batch ingester.
 */

/**
 * Creates the batch upload form.
 *
 * @param array $form
 *   An array representing a form within Drupal.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @return array
 *   An array representing the form to be rendered.
 */
function islandora_paged_content_pdf_batch_batch_form($form, $form_state, AbstractObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/upload_pdf.form');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $applicable_paged_cmodels = islandora_paged_content_retrieve_applicable_cmodels();

  if (isset($object['COLLECTION_POLICY'])) {
    $cp = new CollectionPolicy($object['COLLECTION_POLICY']->content);
    $models = array_intersect_key($applicable_paged_cmodels, $cp->getContentModels());
  }
  else {
    // Not a collection do some shenanigans to find the potential child content
    // models from the registry hook.
    $models = array();
    foreach ($applicable_paged_cmodels as $parent => $values) {
      if (array_intersect(array_keys($values['parents']), $object->models)) {
        $models[$parent] = $parent;
      }
    }
  }
  if (count($models) > 1) {
    $model = FALSE;
  }
  else {
    $model = key($models);
  }
  $form = islandora_paged_content_upload_pdf_form($form, $form_state, $model);
  if (!$model) {
    $form['model'] = array(
      '#type' => 'select',
      '#title' => t('Content model'),
      '#options' => drupal_map_assoc(array_keys($models)),
    );
  }
  $form['namespace'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace for created objects?'),
    '#description' => t('Paged content and page objects will be contructed within this namespace.'),
    '#default_value' => islandora_get_namespace($object->id),
    '#size' => 15,
  );
  unset($form['pdf_file']);
  $scheme = file_default_scheme();
  $form['target'] = array(
    '#type' => 'managed_file',
    '#title' => t('Zip file'),
    '#description' => t('A Zip file containing a number of PDFs.'),
    '#required' => TRUE,
    '#upload_location' => "$scheme://",
    '#upload_validators' => array(
      'file_validate_extensions' => array('zip'),
    ),
    '#weight' => -19,
  );
  $form['parent'] = array(
    '#type' => 'value',
    '#value' => $object->id,
  );
  $form['ingest_set_immediately'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ingest immediately?'),
    '#description' => t('If not selected, objects will just be preprocessed into the queue of objects to be ingested, to be fully-processed (and ingested) later--whenever an admin deems it an appropriate time.'),
    '#default_value' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Ingest'),
  );
  return $form;
}

/**
 * Form submit handler.
 *
 * May kick off a batch process.
 */
function islandora_paged_content_pdf_batch_batch_form_submit($form, $form_state) {
  $target = file_load($form_state['values']['target']);
  $form_state['values']['target'] = $target->uri;
  $connection = islandora_get_tuque_connection();
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $applicable_paged_cmodels = islandora_paged_content_retrieve_applicable_cmodels();
  $parent = islandora_object_load($form_state['values']['parent']);
  $parents = array_intersect_key($applicable_paged_cmodels[$form_state['values']['model']]['parents'], drupal_map_assoc($parent->models));
  $predicate = reset($parents);
  $parameters = array(
    'target' => $form_state['values']['target'],
    'text' => $form_state['values']['extract_text'],
    'language' => $form_state['values']['language'],
    'format' => $form_state['values']['image_format'],
    'resolution' => $form_state['values']['resolution'],
    'content_model' => $form_state['values']['model'],
    'parent' => $form_state['values']['parent'],
    'namespace' => $form_state['values']['namespace'],
    'parent_relationship_uri' => FEDORA_RELS_EXT_URI,
    'parent_relationship_pred' => $predicate,
  );
  $preprocessor = new IslandoraPagedContentPDFBatch($connection, $parameters);
  file_usage_add($target, 'islandora_batch', 'islandora_batch_set', $preprocessor->getSetId());
  if ($form_state['values']['ingest_set_immediately']) {
    islandora_batch_ingest(array(
      'preprocessor' => $preprocessor,
    ));
  }
  else {
    islandora_batch_handle_preprocessor($preprocessor);
  }
}
