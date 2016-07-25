<?php

/**
 * @file
 * Implementation of classes from the batch ingest framework.
 */

/**
 * Batch preprocessor class.
 */
class IslandoraBatchWithDerivsPreprocessor extends IslandoraBatchPreprocessor {

  /**
   * Function to get....
   */
  public function preprocess() {

    $input_directory = $this->parameters['scan_target'];
    $object_dirs = array_diff(scandir($input_directory), array('..', '.'));

    // Accumulate a list of the objects added to the queue.
    $added = array();

    // Check each subdirectory for the existence of the key datastream file
    // and if found, add the object to the batch.
    foreach ($object_dirs as $object_dir) {
      $object_dir_path = $input_directory . DIRECTORY_SEPARATOR . $object_dir;
      // A metadata file for the object is required.
      $key_datastream_path = $object_dir_path . DIRECTORY_SEPARATOR . $this->parameters['key_datastream'] . '.xml';
      if (file_exists($key_datastream_path)) {
        $batch_object = new IslandoraBatchWithDerivsObject($this->connection, $key_datastream_path, $this->parameters);
        // Add the instances to the Islandora batch queue.
        $this->addToDatabase($batch_object);
        $added[] = $batch_object;
      }
    }

    return $added;
  }

}
