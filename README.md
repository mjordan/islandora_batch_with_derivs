# Islandora Batch with Derivatives

Islandora batch module for ingesting objects that have pregenerated derivatives (or, in other words, pregenerated datastreams). The typical use cases are:

1. you have created derivatives outside of Islandora to reduce the amount of time it takes to ingest a large batch
2. you are migrating content into Islandora and can export the content you are migrating from the source platform with datastreams pregenerated.
3. you are migrating content from one Islandora instance to another. This is a special case covered in more detail in the "Moving content between Islandora instances", below.

We need to use a specialized batch ingest module for this because the standard Islandora Batch only allows for two files per object, one .xml file for the MODS or DC and one other file for the OBJ. Islandora Batch with Derivatives allows you to group all of the files corresonding to an object's datastreams (with the exception of RELS-EXT) into a subdirectory, as illustrated below.

The [Islandora Book Batch](https://github.com/Islandora/islandora_book_batch) and [Islandora Newspaper Batch](https://github.com/Islandora/islandora_newspaper_batch) modules allow you to add derivative files to page-level directories, speeding up ingestion of those content types hugely. This module takes the same approach, but for other content models.

## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Batch](https://github.com/Islandora/islandora_batch)

## Usage

Enable this module, then run its drush command to import objects:

`drush --user=admin islandora_batch_with_derivs_preprocess --key_datastream=MODS --scan_target=/path/to/object/files --namespace=mynamespace --parent=islandora:mycollection`

Then, to perform the ingest:

`drush --user=admin islandora_batch_ingest`

## Preparing Islandora for ingesting

When using this batch module, you usually want to turn Islandora's derivative creation off. To do this, got to Admin > Islandora > Configuration, and check "Defer derivative generation during ingest". This will disable all derivative generation. You should probably return this setting to its original value after your batch finishes running.

## Preparing your content files for ingesting

This batch module uses filenames to identify the files that correspond to specific datastreams. All of the files you are ingesting with an object should go in one directory (a subdirectory of the path you identify in the drush command with the `--scan_target` option). Each object-level subdirectory must have at least a file for the "key datastream", which is either the MODS (MODS.xml) or DC (DC.xml) datastream. This datastream is identified in the `--key_datastream` option. All other datastream files are optional.

Some points to note:

* The objects ingested by this batch module are assigned PIDs by the destination Islandora. PIDs in datastreams such as DC and RELS-EXT are not reused.
* Related to the previous point, you would not typically pregenerate the RELS-EXT datastream, since it contains data expressing the relationships between the object and other objects. It is created automatically on ingest regardless of whether "Defer derivative generation during ingest" is enabled.
* The label applied to each object is derived from the MODS `<title>` element, or if there is no MODS.xml file in the object directory, from the DC `<title>` element.
* If MODS is specified as the value of `--key_datastream`, the DC datastream that is generated for each object will be generated from the MODS.xml file, which is Islandora's default behavior. If you prefer that a default DC datastream containing only a title and an identifier element be generated, include the `--create_dc_from_mods=false` option in the `islandora_batch_with_derivs_preprocess` command (e.g., `islandora_batch_with_derivs_preprocess --key_datastream=MODS --create_dc_from_mods=false`). If both MODS.xml and DC.xml exist in the object's input directory, both datastreams are populated from the files.
* By default, the content model of each object is derived from the extension of the file named 'OBJ'. If any of the objects you are ingesting do not have an OBJ datastream file, you will need to specify a content model for them with the `--content_models` option. Note that the specificed content model must apply to all objects in the current batch.
* Even though all datastream files other than either MODS.xml or DC.xml are optional, if you enable "Defer derivative generation during ingest", Islandora will not create the missing derivatives. For every derivative file that you do not include, you will need to generate the corresponding derivatives later.

### Example input directories

Each object in the batch must be in its own subdirectory under the path specified in `--scan_target`. Within each object directory are all the files that will be used to create that object's datastreams, named using datastream IDs:

```
/tmp/valueofscantarget
├── foo 
│   ├── DC.xml
│   ├── MEDIUM_SIZE.jpg
│   ├── MODS.xml
│   ├── OBJ.jpg
│   ├── TECHMD.xml
│   └── TN.jpg
├── bar
│   ├── DC.xml
│   ├── MEDIUM_SIZE.jpg
│   ├── MODS.xml
│   ├── OBJ.jpg
│   ├── TECHMD.xml
│   └── TN.jpg
└── baz
    ├── DC.xml
    ├── MEDIUM_SIZE.jpg
    ├── MODS.xml
    ├── OBJ.jpg
    ├── TECHMD.xml
    └── TN.jpg
```

The names of the object subdirectories have no significance (unless the `--use_pids=true` option is included, as described below).

## Moving content between Islandora instances

It is possible to use this module to ingest content that originated in another Islandora instance. You can choose to allow the target Islandora to generate new PIDs for the ingested objects, or you can choose to assign specific PIDs to the new objects. A common instance of the latter case is to assign the PIDs assigned by another Islandora instance. 

### Assigning new PIDS

The default behavior of this module is to allow the target Islandora to assign PIDs to ingested objects. The most important implication of this is that Islandora will generate a new RELS-EXT datastream for each object, containing only the RDF statements that are generated on ingest (collecion membership and content model). If a RELS-EXT datastream file is present, it will be ignored. The namespace of the new PIDs will be the one specified in the `islandora_batch_with_derivs_preprocess` command's `--namespace` option.

### Preserving existing PIDS

PIDs can be assigned to the objects being ingested through use of the `islandora_batch_with_derivs_preprocess` command's `--use_pids` option. If this option has a value of 'true' (e.g., `--use_pids=true`), the name of each object-level directory containing the datastream files will be converted to a PID and that PID will be assigned to the resulting object. The colon in the PID must be represented by a plus ssign (`+`); this will be converted into the colon in the preserved PID. For example, with the `--use_pids=true`, the following directory structure will result in objects with PIDs 'foo:23', 'bar:2', and 'baz:special_object':

```
/tmp/valueofscantarget
├── foo+23 
│   ├── DC.xml
│   ├── MEDIUM_SIZE.jpg
│   ├── MODS.xml
│   ├── OBJ.jpg
│   ├── RELS-EXT.rdf
│   ├── TECHMD.xml
│   └── TN.jpg
├── bar+2
│   ├── DC.xml
│   ├── MEDIUM_SIZE.jpg
│   ├── MODS.xml
│   ├── OBJ.jpg
│   ├── RELS-EXT.rdf
│   ├── TECHMD.xml
│   └── TN.jpg
└── baz+special_object
    ├── DC.xml
    ├── MEDIUM_SIZE.jpg
    ├── MODS.xml
    ├── OBJ.jpg
│   ├── RELS-EXT.rdf
    ├── TECHMD.xml
    └── TN.jpg
```

If a RELS-EXT datastream file exists in an object directory, the relationships expressed in it will be parsed out, and all new relationships resulting from the ingest (e.g., additional collection membership) will be added to the object's RELS-EXT datastream with the exception of duplicate 'isMemberOfCollection' relationships. If no RELS-EXT datastream file exists, Islandora will generate one.

One strategy for migrating objects, with their relationships intact, is to export objects using the [Islandora Dump Datastreams](https://github.com/mjordan/islandora_dump_datastreams) module and then ingest the resulting packages as described in this section.

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## Development and feedback

Pull requests are welcome, as are use cases and suggestions.

## License

 [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
