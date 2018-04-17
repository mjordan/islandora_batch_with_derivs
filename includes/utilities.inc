<?php

/**
 * @file
 * Utility functions.
 */

/**
 * Determines content model based on file extension.
 *
 * @param string $extension
 *   The file extension of the OBJ datastream.
 *
 * @return string
 *   A content model PID.
 */
function islandora_batch_with_derivs_get_cmodel($extension) {
  $map = array(
    'jpeg' => 'islandora:sp_basic_image',
    'jpg' => 'islandora:sp_basic_image',
    'gif' => 'islandora:sp_basic_image',
    'png' => 'islandora:sp_basic_image',
    'tif' => 'islandora:sp_large_image_cmodel',
    'tiff' => 'islandora:sp_large_image_cmodel',
    'jp2' => 'islandora:sp_large_image_cmodel',
    'pdf' => 'islandora:sp_pdf',
    'mp3' => 'islandora:sp-audioCModel',
    'mp4a' => 'islandora:sp-audioCModel',
    'm4a' => 'islandora:sp-audioCModel',
    'oga' => 'islandora:sp-audioCModel',
    'ogg' => 'islandora:sp-audioCModel',
    'flac' => 'islandora:sp-audioCModel',
    'wav' => 'islandora:sp-audioCModel',
    'mp4' => 'islandora:sp_videoCModel',
    'm4v'  => 'islandora:sp_videoCModel',
    'mkv'  => 'islandora:sp_videoCModel',
    'mpeg' => 'islandora:sp_videoCModel',
    'mpe' => 'islandora:sp_videoCModel',
    'mpg' => 'islandora:sp_videoCModel',
    'qt' => 'islandora:sp_videoCModel',
    'mov' => 'islandora:sp_videoCModel',
    'ogv' => 'islandora:sp_videoCModel',
  );
  return $map[$extension];
}

/**
 * Gets the object label from the key datastream.
 *
 * @param string $xml
 *   The contents of the MODS.xml, MADS.xml, or DC.xml file.
 *
 * @return string
 *   A label for the object.
 */
function islandora_batch_with_derivs_get_label($xml, $ns, $ns_uri, $xpath_query) {
  $dom = new DOMDocument();
  $dom->loadXML($xml);
  $xpath = new DOMXPath($dom);
  $xpath->registerNamespace($ns, $ns_uri);
  $labels = $xpath->query($xpath_query);
  if ($labels->length > 0) {
    $label = $labels->item(0)->nodeValue;
  }
  else {
    $label = '[No object label available]';
  }
  return $label;
}

/**
 * Generates DC XML from XML via XSLT (e.g., MODS or MADS).
 *
 * @param string $xml
 *   An XML string.
 * @param string $path_to_xslt
 *   Path to the XSLT stylesheet.
 *
 * @return string
 *   The transformed DC XML.
 */
function islandora_batch_with_derivs_get_dc_from_xml($xml, $path_to_xslt) {
  $xsl_doc = new DOMDocument();
  $xsl_doc->load($path_to_xslt);
  $xml_doc = new DOMDocument();
  $xml_doc->loadXML($xml);
  $xslt_proc = new XSLTProcessor();
  $xslt_proc->importStylesheet($xsl_doc);
  $dc_xml = $xslt_proc->transformToXML($xml_doc);
  return $dc_xml;
}
