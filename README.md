# Islandora Batch with Derivatives

Islandora batch module for ingesting objects that have pregenerated derivatives (or, in other words, pregenerated datastreams). The typical use cases are:

1. you have created derivatives outside of Islandora to reduce the amount of time it takes to ingest a large batch
2. you are migrating content into Islandora and can export the content you are migrating from the source platform with datastreams pregenerated.

We need to use a specialized batch ingest module for this because the standard Islandora Batch only allows for two files per object, one .xml file for the MODS or DC and one other file for the OBJ. This batch module allows you to group all of the files corresonding to an object's datastreams (with the exception of RELS-EXT) into a subdirectory, as illustrated below.

The [Islandora Book Batch](https://github.com/Islandora/islandora_book_batch) and [Islandora Newspaper Batch](https://github.com/Islandora/islandora_newspaper_batch) modules allow you to add derivative files to page-level directories, speeding up ingestion of those content types hugely. This module takes the same approach, but for other content models.

## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Batch](https://github.com/Islandora/islandora_batch)

## Usage

Enable this module, then run its drush command to import objects:

`drush --user=admin islandora_batch_with_derivs_preprocess --key_datastream=MODS --scan_target=/path/to/object/files --namespace=mynamespace --content_models=islandora:sp_large_image_cmodel --parent=islandora:mycollection`

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
* By default, the content model of each object is derived from the extension of the file named 'OBJ'. If any of the objects you are ingesting do not have an OBJ datastream file, you will need to specify a content model for them with the `--content_models` option.
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

The names of the object subdirectories have no significance.

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## Development and feedback

Pull requests are welcome, as are use cases and suggestions.

## License

 [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
