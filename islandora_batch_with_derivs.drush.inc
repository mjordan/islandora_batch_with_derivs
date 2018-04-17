<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_batch_with_derivs_drush_command() {
  $items = array();

  $items['islandora_batch_with_derivs_preprocess'] = array(
    'aliases' => array('ibwd'),
    'description' => 'Preprocess Islandora objects with derivatives into batch queue entries.',
    'drupal dependencies' => array('islandora_batch'),
    'options' => array(
      'scan_target' => array(
        'description' => 'The target directory to scan.',
        'required' => TRUE,
      ),
      'namespace' => array(
        'description' => 'The namespace for objects created by this command.',
        'required' => TRUE,
      ),
      'parent' => array(
        'description' => 'The collection to which the generated items should be added.',
        'required' => TRUE,
      ),
      'key_datastream' => array(
        'description' => 'The datastream ID that will be used to identify the object. "MODS" or "DC" are valid choices.',
        'required' => TRUE,
      ),
      'parent_relationship_uri' => array(
        'description' => 'The namespace URI of the relationship to the parent. Defaults to "info:fedora/fedora-system:def/relations-external#".',
        'value' => 'optional',
      ),
      'parent_relationship_pred' => array(
        'description' => 'The predicate of the relationship to the parent. Defaults to "isMemberOf".',
        'value' => 'optional',
      ),
      'content_models' => array(
        'description' => 'The content model to use for objects that do not have an OBJ derivative file. If there is an OBJ file, the provided content model overrides the automatically assigned one.',
        'value' => 'optional',
      ),
      'create_dc' => array(
        'description' => "If true, the DC datastream will be generated from the MODS.xml or MAds.xml key datastream. If false, the DC datastream will be generated from the object's label and PID.",
        'value' => 'required',
      ),
      'use_pids' => array(
        'description' => "If true, PIDs derived from the object directory names will be assigned. The colon (:) in the PID should be represented by a plus sign (+) in the directory name, and the '--namespace' option will be ignored. If false (the default), objects will be assigned new PIDs. ",
        'value' => 'required',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  $items['islandora_batch_with_derivs_check_pids'] = array(
    'aliases' => array('ibwdcp'),
    'description' => "Scans the input directory and reports whether PIDs of new objects already exist. Useful in preparation for using islandora_batch_with_derivs_preprocess's --use_pids option.",
    'options' => array(
      'scan_target' => array(
        'description' => 'The target directory to scan.',
        'required' => TRUE,
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );


  return $items;
}

/**
 * Implements hook_drush_command().
 */
function drush_islandora_batch_with_derivs_preprocess() {
  // Due to how Drush bootstrapping works, the connection may be created
  // without credentials (when your site's front page is
  // 'islandora/object/some:object', for example). Resetting to ensure a new
  // connection gets created should fix it.
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  $parameters = array(
    'namespace' => drush_get_option('namespace'),
    'scan_target' => drush_get_option('scan_target'),
    'key_datastream' => drush_get_option('key_datastream'),
    'content_models' => drush_get_option('content_models'),
    'parent' => drush_get_option('parent'),
    'parent_relationship_uri' => drush_get_option('parent_relationship_uri', 'info:fedora/fedora-system:def/relations-external#'),
    'parent_relationship_pred' => drush_get_option('parent_relationship_pred', 'isMemberOf'),
    'create_dc' => drush_get_option('create_dc', TRUE),
    'use_pids' => drush_get_option('use_pids', FALSE),
  );

  if ($parameters['key_datastream'] == 'MADS') {
    if (!module_exists('islandora_entities')) {
      return drush_set_error('Using MADS as the key datastream requires the Islandora Entities Solution Pack');
    }
  }

  // Run the preprocessor.
  $preprocessor = new IslandoraBatchWithDerivsPreprocessor($connection, $parameters);
  $preprocessed = islandora_batch_handle_preprocessor($preprocessor);
  drush_log(t("SetId: @s", array('@s' => $preprocessor->getSetId())), "ok");
}

/**
 * Implements hook_drush_command().
 */
function drush_islandora_batch_with_derivs_check_pids() {
  $input_directory = drush_get_option('scan_target');
  $object_dirs = array_diff(scandir($input_directory), array('..', '.'));
  foreach ($object_dirs as $object_dir) {
    if (preg_match('/\+/', $object_dir)) {
      $pid = preg_replace('/\+/', ':', $object_dir);
      // Check whether an object with PID exists, and if so,
      // report that the object will be skipping on ingest.
      if (islandora_object_load($pid)) {
        drush_log(dt("An object with !pid already exists; object in !object_dir will not be ingested.",
          array('!pid' => $pid, '!object_dir' => $input_directory . DIRECTORY_SEPARATOR . $object_dir)), 'warning');
      }
    }
  }
}

/**
 * Parent collection validation.
 */
function drush_islandora_batch_with_derivs_preprocess_validate() {
  $parent = drush_get_option('parent');
  $object = islandora_object_load($parent);
  if (!$object) {
    return drush_set_error('This is not a valid parent', dt('The specified parent (@parent) was unable to be loaded.',
      array('@parent' => $parent)));
  }
}
