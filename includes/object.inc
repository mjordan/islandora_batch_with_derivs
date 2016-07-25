<?php

/**
 * @file
 * Implementation of classes from the batch ingest framework.
 */

/**
 * Batch object class.
 */
class IslandoraBatchWithDerivsObject extends IslandoraBatchObject {
  protected $xmlObjFilePath;
  protected $preprocessorParameters;

  /**
   * Constructor.
   */
  public function __construct(IslandoraTuque $connection, string $key_datastream_path, $batch_parameters) {
    parent::__construct(NULL, $connection->repository);

    $this->keyDsFilePath = $key_datastream_path;
    $this->objectContentPath = dirname($key_datastream_path);
    $this->preprocessorParameters = $batch_parameters;
  }

  /**
   * Assemble the base datastreams.
   */
  public function batchProcess() {
    // Get datastreams.
    $datastreams = $this->getDatastreams();
    if (count($datastreams)) {
      foreach ($datastreams as $datastream) {
        $this->ingestDatastream($datastream);
      }
    }

    module_load_include('inc', 'islandora_batch_with_derivs', 'includes/utilities');
    if ($this->preprocessorParameters['key_datastream'] == 'MODS') {
      $mods_content = file_get_contents($this->keyDsFilePath);
      $this->label = islandora_batch_with_derivs_get_label_from_mods($mods_content);
    }
    if ($this->preprocessorParameters['key_datastream'] == 'DC') {
      $dc_content = file_get_contents($this->keyDsFilePath);
      $this->label = islandora_batch_with_derivs_get_label_from_dc($dc_content);
    }

    $this->addRelationships();

    // Indicate that this object is ready to be ingested.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Add relationships.
   */
  public function addRelationships() {
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $this->preprocessorParameters['parent']);
    // Get content model from OBJ file extension.
    module_load_include('inc', 'islandora_batch_with_derivs', 'includes/utilities');
    $obj_path_pattern = $this->objectContentPath . DIRECTORY_SEPARATOR . 'OBJ';
    $obj_files = glob($obj_path_pattern . '*');
    // There should be only one OBJ file, or none (since the OBJ datastream
    // is optional).
    if (count($obj_files)) {
      $ext = pathinfo($obj_files[0], PATHINFO_EXTENSION);
      $this->models = islandora_batch_with_derivs_get_cmodel($ext);
    }
    else {
      // If there is no OBJ to get the content model from, use the value
      // of the --content_models option. @todo: account for when this
      // option is missing.
      $content_model = $this->preprocessorParameters['content_models'];
      $this->models = islandora_batch_with_derivs_get_cmodel($content_model);
    }
  }

  /**
   * Get a list of resources.
   */
  public function getResources() {
    return array();
  }

  /**
   * Creates datastreams for the object based on filename.
   *
   * Looks inside the object's content directory and adds a datastream
   * for each file it finds there.
   *
   * @return array
   *   A list of datastreams to ingest with the current object.
   */
  private function getDatastreams() {
    $datastreams = array();

    $ds_filepaths = glob($this->objectContentPath . DIRECTORY_SEPARATOR . '*');
    foreach ($ds_filepaths as $ds_filepath) {
      $dsid = pathinfo($ds_filepath, PATHINFO_FILENAME);
      $dsid = strtoupper($dsid);
      if (strlen($dsid)) {
        $ds = $this->constructDatastream($dsid, 'M');
        $mime_detector = new MimeDetect();
        $ds->mimetype = $mime_detector->getMimetype($ds_filepath);
        $ds->label = $dsid . ' datastream';
        $ds->setContentFromFile($ds_filepath, FALSE);
        $datastreams[] = $ds;
      }
    }
    return $datastreams;
  }

}
