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
   * Generates objects from input files and adds them to the batch queue.
   */
  public function preprocess() {
    $input_directory = $this->parameters['scan_target'];
    $object_dirs = array_diff(scandir($input_directory), array('..', '.'));

    // Accumulate a list of the objects to be added to the queue.
    $added = array();

    // Check each subdirectory for the existence of the key datastream file
    // and if found, add the object to the batch.
    foreach ($object_dirs as $object_dir) {
      $object_dir_path = $input_directory . DIRECTORY_SEPARATOR . $object_dir;
      // A metadata file for the object is required (either MODS or DC).
      $key_datastream_path = $object_dir_path . DIRECTORY_SEPARATOR . $this->parameters['key_datastream'] . '.xml';
      if (file_exists($key_datastream_path)) {
        $batch_object = new IslandoraBatchWithDerivsObject($this->connection, $key_datastream_path, $this->parameters);
        if ($this->parameters['use_pids']) {
          // Get PID from object directory name. We use '+' in directory names
          // to represent the PID's ':' because that character is guaranteed
          // to not exist in the PID (it's not a valid character in PIDs) and
          // it is filesystem safe.
          if (preg_match('/\+/', $object_dir)) {
            $pid = preg_replace('/\+/', ':', $object_dir);
            // Check whether an object with PID exists, and if so,
            // report that we are skipping ingest on this object.
            if (islandora_object_load($pid)) {
              // @todo: This check covers the case where an object that has the
              // incoming PID already exists, but we also need to check the
              // {islandora_batch_queue} table. If a previous batch ingest
              // failed and the incoming PID existed in that failed batch,
              // a subsequent batch ingest containing the PID will fail because
              // of integrity errors on this table.
              watchdog('islandora_batch_with_derivs', 'An object with the !pid already exists, skipping ingest.',
                array('!pid' => $pid), WATCHDOG_WARNING);
            }
            else {
              // Add the instance to the Islandora batch queue using the
              // existing PID.
              $batch_object->id = $pid;
              $this->addToDatabase($batch_object);
              $added[] = $batch_object;
            }
          }
          else {
            // If we can't get a PID from the object directory name, report
            // that we are skipping inigest on this object.
            watchdog('islandora_batch_with_derivs', 'PID not available from object directory name !object_dir, skipping ingest.',
              array('!object_dir' => $object_dir), WATCHDOG_WARNING);
          }
        }
        else {
          // Add the instance to the Islandora batch queue. Islandora will
          // assign a PID.
          $this->addToDatabase($batch_object);
          $added[] = $batch_object;
        }
      }
    }

    return $added;
  }

}
