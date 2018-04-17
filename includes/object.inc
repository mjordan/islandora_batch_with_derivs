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
  public function __construct(IslandoraTuque $connection, $key_datastream_path, $batch_parameters) {
    parent::__construct(NULL, $connection->repository);
    $this->keyDsFilePath = $key_datastream_path;
    $this->objectContentPath = dirname($key_datastream_path);
    $this->preprocessorParameters = $batch_parameters;
    if ($this->preprocessorParameters['create_dc'] && $this->preprocessorParameters['key_datastream'] == 'MODS') {
      $this->keyToDcTransform = drupal_get_path('module', 'islandora_batch') . '/transforms/mods_to_dc.xsl';
    }
    if ($this->preprocessorParameters['create_dc'] && $this->preprocessorParameters['key_datastream'] == 'MADS') {
      $this->keyToDcTransform = drupal_get_path('module', 'islandora_entities') . '/xml/mads_to_dc.xsl';
    }
  }

  /**
   * Assemble the datastreams and relationships.
   */
  public function batchProcess() {
    $datastreams = $this->getDatastreams();
    if (count($datastreams)) {
      foreach ($datastreams as $datastream) {
        $this->ingestDatastream($datastream);
      }
    }

    module_load_include('inc', 'islandora_batch_with_derivs', 'includes/utilities');
    if ($this->preprocessorParameters['key_datastream'] == 'MODS') {
      $mods_content = file_get_contents($this->keyDsFilePath);
      $this->label = islandora_batch_with_derivs_get_label($mods_content, 'mods', 'http://www.loc.gov/mods/v3', '//mods:titleInfo/mods:title');
      // Datastreams in the new object correspond 1:1 to the files in the input directory. DC is an
      // exception: if the Drush --create_dc is true (default) and MODS is the key datastream,
      // a DC datastream will be generated from the MODS.xml file. If this option is false, a default DC
      // datastream is generated from the object label and PID. If false and both a MODS.xml and DC.xml
      // datastream file exist, both files will populate their respective datastreams (in other words,
      // DC is not generated from MODS).
      if ($this->preprocessorParameters['create_dc'] === TRUE && !$datastreams['DC']) {
        $dc_content = islandora_batch_with_derivs_get_dc_from_xml($mods_content, $this->keyToDcTransform);
        $dc = $this->constructDatastream('DC', 'X');
        $dc->mimetype = 'text/xml';
        $dc->label = 'Dublin Core Record for this object';
        $dc->setContentFromString($dc_content);
        $this->ingestDatastream($dc);
      }
    }
    if ($this->preprocessorParameters['key_datastream'] == 'MADS') {
      $mads_content = file_get_contents($this->keyDsFilePath);
      $this->label = islandora_batch_with_derivs_get_label($mads_content, 'mads', 'http://www.loc.gov/mads/v2', '//mads:authority/mads:titleInfo/mads:title');
      if ($this->preprocessorParameters['create_dc'] === TRUE && !$datastreams['DC']) {
        $dc_content = islandora_batch_with_derivs_get_dc_from_xml($mads_content, $this->keyToDcTransform);
        $dc = $this->constructDatastream('DC', 'X');
        $dc->mimetype = 'text/xml';
        $dc->label = 'Dublin Core Record for this object';
        $dc->setContentFromString($dc_content);
        $this->ingestDatastream($dc);
      }
    }
    if ($this->preprocessorParameters['key_datastream'] == 'DC') {
      $dc_content = file_get_contents($this->keyDsFilePath);
      $this->label = islandora_batch_with_derivs_get_label($dc_content, 'dc', 'http://purl.org/dc/elements/1.1/', '//dc:title');
    }

    $this->addRelationships();

    // Indicate that this object is ready to be ingested.
    return ISLANDORA_BATCH_STATE__DONE;
  }

  /**
   * Add relationships.
   */
  public function addRelationships() {
    module_load_include('inc', 'islandora_batch_with_derivs', 'includes/utilities');
    $this->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $this->preprocessorParameters['parent']);
    $rels = $this->getRelationships($this->objectContentPath);
    $add_models = TRUE;
    foreach ($rels as &$rel) {
      // If any fedora-model:hasModel relationships exist, do not assign
      // $this->models relationships below.
      if ($rel['namespace_prefix'] == 'fedora-model' && $rel['predicate'] == 'hasModel') {
        $rel['object'] = 'info:fedora/' . $rel['object'];
        $add_models = FALSE;
      }

      // We don't want to add duplicate isMemberOfCollection relationships.
      if ($rel['predicate'] != 'isMemberOfCollection' && $rel['object'] != $this->preprocessorParameters['parent']) {
        $this->relationships->add($rel['namespace_uri'], $rel['predicate'], $rel['object']);
      }
    }

    // Get content model from OBJ file extension. Or, if the content model is
    // provided as a Drush option, use it instead.
    $obj_path_pattern = $this->objectContentPath . DIRECTORY_SEPARATOR . 'OBJ';
    $obj_files = glob($obj_path_pattern . '*');
    // There should be only one OBJ file, or none (since the OBJ datastream
    // is optional).
    if (count($obj_files) <= 1) {
      if ($add_models) {
        // A content model was not provided as a Drush option. Use the OBJ
        // file's extension.
        if (is_null($this->preprocessorParameters['content_models'])) {
          $ext = pathinfo($obj_files[0], PATHINFO_EXTENSION);
          $this->models = islandora_batch_with_derivs_get_cmodel($ext);
        }
        // A content model was provided as a Drush option. Use it instead.
        else {
          $this->models = $this->preprocessorParameters['content_models'];
        }
      }
    }
    else {
      // If there is no OBJ to get the content model from, use the value
      // of the --content_models option. @todo: account for when this
      // option is missing.
      if ($add_models) {
        $this->models = $this->preprocessorParameters['content_models'];
      }
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
      if (strlen($dsid) && $dsid != 'RELS-EXT') {
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

  /**
   * Parses relationships out of a RELS-EXT datastream file.
   *
   * We can't ingest the RELS-EXT file directly because we may get duplicate
   * relationships, e.g. the source object's RELS-EXT and the relationships
   * defined on ingest may contain the same relationships.
   *
   * @param $string $path
   *   The path to the object directory.
   *
   * @return array
   *   An associative array of relationships parsed out from the RELS-EXT file. 
   */
  private function getRelationships($path) {
    $rels = array();
    if (!$this->preprocessorParameters['use_pids']) {
      return $rels;
    }
    // Get path to the RELS-EXT datastream file.
    $rels_ext_path_pattern = $path . DIRECTORY_SEPARATOR . 'RELS-EXT';
    $rels_ext_files = glob($rels_ext_path_pattern . '*');
    // There should be only RELS-EXT file, or none.
    if (count($rels_ext_files) === 1) {
      $doc = new DOMDocument();
      $doc->load($rels_ext_files[0]);
      $desc = $doc->getElementsByTagName('Description')->item(0);
      foreach ($desc->childNodes as $node) {
        if (property_exists($node, 'tagName')) {
          list($prefix, $tag) = explode(':', $node->tagName);
          $rel = array();
          $rel['predicate'] = $tag; 
          $object_value = preg_replace('@info:fedora/@', '', $node->getAttribute('rdf:resource'));
          $rel['object'] = $object_value; 
          $rel['namespace_prefix'] = $prefix;
          $rel['namespace_uri'] = $node->lookupnamespaceURI($prefix);
          $rels[] = $rel;
        }
      }
    }
    return $rels;
  }

}
