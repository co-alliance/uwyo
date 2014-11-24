<?php

/*
 *
 * MSU_newspaper_migration_create_structure.php
 * php script written by PALS                                                    
 * creates directory structure with appropriate file names for input into
 * pals_newspaper_batch_preprocess and converts tab-delimited metadata to MODS 
 * Structure is as follows:
 * /Issue-number/
 *    MODS.xml
 *    1/
 *       OBJ.tiff
 *    2/
 *       OBJ.tiff
 * where directories 1, 2, etc. are created for each page object (OBJ.tiff)
 */


function createMODSFile($issue_directory, $issue_value, $log_handle) {
/*
 *
 * MSU_newspaper_migration_from_CONTENTdm.php
 * input is tab-delimited file of metadata
 * output is MODS metadata file for each issue 
 */

$patterns = array();
$patterns[0] = '/&/';
$patterns[1] = '/</';
$patterns[2] = '/>/';
$replacements = array();
$replacements[0] = '';
$replacements[1] = '';
$replacements[2] = '';

$file = $issue_directory . "/" . $issue_value . ".txt";
$handle = fopen($file, "r");

// Throw away header line
$data = fgetcsv($handle, 0, "\t");

//Read first data line, which has the title of the issue
$data_one = fgetcsv($handle, 0, "\t");

//Read second data line, which has the rest of the metadata
$data_two = fgetcsv($handle, 0, "\t");


   $output_file = $issue_directory . "/MODS.xml"; //create file with name equal to MDL identifier
   $output_handle = fopen($output_file, "w");
   if ($output_handle === FALSE) {
      echo "output handle is false; couldn't open output file";
      }

// Write header to file
      fwrite($output_handle, "<?xml version=\"1.0\"?>\n");
      fwrite($output_handle, "<mods xmlns=\"http://www.loc.gov/mods/v3\" xmlns:mods=\"http://www.loc.gov/mods/v3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");

// Process field 0 of the first line = title
      if ($data_one[0] !== "") {
        $data_one[0] = preg_replace($patterns, $replacements, $data_one[0]);  //remove special characters not acceptable in xml
        fwrite($log_handle, "Processing $data_two[24]\n");
        fwrite($output_handle, "<titleInfo>\n");
        if (strlen($data_one[0]) > 255) {
           fwrite($log_handle, "Title too long: " . $data_one[0] . "\n");
           fwrite($output_handle,  "<title>" . substr($data_one[0],0,255) . "</title>\n");
        }
        else {
           fwrite($output_handle,  "<title>" . $data_one[0] . "</title>\n");
        }
        fwrite($output_handle, "</titleInfo>\n");
      }
      else {
           fwrite($log_handle, "Title missing: " . $data_two[24] . "\n");
      }

 //Process field 43 = Object File Name
//      fwrite($output_handle, "<relatedItem type=\"original\">\n");
//      fwrite($output_handle, "<titleInfo>\n");
//      if ($data[43] !== "") {
//          fwrite($output_handle,  "<title>" . $data[43] . "</title>\n");
//      }
//      else {   // if blank, use MDL identifer followed by .tif extension
//          fwrite($output_handle,  "<title>" . $data[25] . ".tif</title>\n");
//      }
//      fwrite($output_handle, "</titleInfo>\n");
//      fwrite($output_handle, "</relatedItem>\n");

// Process field 24 = Local Identifier
      fwrite($output_handle, "<identifier type = \"local\">" . substr($data_two[23],0,11) . substr($data_two[24],3,5) . "</identifier>\n");

// Process field 25 = MDL Identifier - use first 8 characters
      fwrite($output_handle, "<identifier type = \"MDL\">" . substr($data_two[24],0,8) . "</identifier>\n");

//      if ($data[1] !== "") {
// Process field 1 = Creator
//         fwrite($output_handle, "<name type=\"creator\">\n");
//         fwrite($output_handle, "<namePart>" . trim($data[1], ";") . "</namePart>\n");
//         fwrite($output_handle, "<role>\n");
//         fwrite($output_handle, "<roleTerm type=\"text\">creator</roleTerm>\n");
//         fwrite($output_handle, "</role>\n");
//         fwrite($output_handle, "</name>\n");
//         }

//      if ($data[2] !== "") {
// Process field 2 = Contributor
//      fwrite($output_handle, "<name type=\"contributor\">\n");
//      fwrite($output_handle, "<namePart>" . $data[2] . "</namePart>\n");
//      fwrite($output_handle, "<role>\n");
//      fwrite($output_handle, "<roleTerm type=\"text\">contributor</roleTerm>\n");
//      fwrite($output_handle, "</role>\n");
//      fwrite($output_handle, "</name>\n");
//      }

// Process field 22= Contributing Organization; decided to make them all the same value
      fwrite($output_handle, "<name type=\"contrib_org\">\n");
      fwrite($output_handle, "<namePart>University Archives and Southern Minnesota Historical Center, Memorial Library, Minnesota State University, Mankato</namePart>\n");
      fwrite($output_handle, "<description>University Archives and Southern Minnesota Historical Center, Memorial Library, Minnesota State University, Mankato, P.O. Box 8419, Mankato, MN  56002-8419. http://lib.mnsu.edu/archives/</description>\n");
      fwrite($output_handle, "<role>\n");
      fwrite($output_handle, "<roleTerm type=\"text\">contributing organization</roleTerm>\n");
      fwrite($output_handle, "</role>\n");
      fwrite($output_handle, "</name>\n");

//      if ($data[3] !== "") {
// Process field 3 = Description
//      $data[3] = preg_replace($patterns, $replacements, $data[3]);  //remove special characters not acceptable in xml
//      fwrite($output_handle, "<note type = \"description\">" . $data[3] . "</note>\n");
//      }

      fwrite($output_handle, "<originInfo>\n");
// Use date information in local id for Date of Creation
//
      $date_created = substr($data_two[23],11);
      $date_created = substr($date_created,strpos($date_created,"-")+1);
//      fwrite($output_handle, "<dateCreated>" . $date_created . "</dateCreated>\n");

//  ***Need dateIssued value so it loads issue correctly
      fwrite($output_handle, "<dateIssued>" . $date_created . "</dateIssued>\n");

//       fwrite($output_handle, "<publisher>" . $data[5] . "</publisher>\n");

// Process field 26 = Date Digital
      if ($data_two[26] !== "") {
         $date = new DateTime($data_two[26]);  //format date as yyyy-mm-dd
         fwrite($output_handle, "<dateOther>" .  $date->format('Y-m-d') . "</dateOther>\n");
      }
      fwrite($output_handle, "</originInfo>\n");

// Process field 6 = Dimensions, field 31 = Master File Format, field 9 = Item Physical Format and field 30 = Access File Name (Item Digital Format)
      fwrite($output_handle, "<physicalDescription>\n");
//      if ($data[6] !== "") {
//         fwrite($output_handle, "<extent>" . trim($data[6], ";") . "</extent>\n");
//      }
      fwrite($output_handle, "<internetMediaType>Image/tiff</internetMediaType>\n");
      fwrite($output_handle, "<form>Newspapers</form>\n");
      fwrite($output_handle, "<note>Image/jp2</note>\n");
      fwrite($output_handle, "</physicalDescription>\n");

//      if ($data[42]!== "") {
// Process field 42 = Location
//      fwrite($output_handle,  "<location><physicalLocation>" . $data[42] . "</physicalLocation></location>\n");
//      }     

// Process field 8 = Item Type
      fwrite($output_handle,  "<typeOfResource>Text</typeOfResource>\n");

// Process subject fields
      fwrite($output_handle, "<subject>\n");

//Process field 7 = Minnesota Reflections Topic
      fwrite($output_handle, "<topic authority = \"mdl\">" . "Education" . "</topic>\n");

//      if ($data[10]!== "") {
//Process field 10 = Formal MDL subject
//      fwrite($output_handle, "<topic authority = \"lcsh\">" . $data[10] . "</topic>\n");
//         }

//      if ($data[11] !== "") {
//Process field 11 = Local subject
//         $token = strtok($data[11], ";");

//         while ($token != false) {
            fwrite($output_handle, "<topic authority = \"msu\">" . "Mankato State Normal School" . "</topic>\n");
            fwrite($output_handle, "<topic authority = \"msu\">" . "College Student Newspapers and Periodicals-Minnesota-Mankato" . "</topic>\n");
            fwrite($output_handle, "<topic authority = \"msu\">" . "College Students" . "</topic>\n");
//            $token = strtok(";");
//            } 
//         }

//Process hierarchicalGeographic fields
      fwrite($output_handle, "<hierarchicalGeographic>\n");

//Process field 12 = City
      fwrite($output_handle, "<city>Mankato</city>\n");

      if ($data_two[13] !== "") {
//Process field 13 = City district
      fwrite($output_handle, "<citySection>" . $data_two[13] . "</citySection>\n");
      }

//      if ($data[14] !== "") {
//Process field 14 = County
      fwrite($output_handle, "<county>Blue Earth</county>\n");
//      }

//Process field 15 = State or Province
      fwrite($output_handle, "<state>Minnesota</state>\n");

//Process field 16 = Country
      fwrite($output_handle, "<country>United States</country>\n");

      fwrite($output_handle, "</hierarchicalGeographic>\n");

//      if ($data[17] !== "") {
//Process field 17 = Geographic Feature
//      fwrite($output_handle, "<geographic>" . $data[17] . "</geographic>\n");
//      }


//Process latitude and longitude
      fwrite($output_handle, "<extension type=\"coordinates\">\n");
      fwrite($output_handle, "<geo.lat>44.162085</geo.lat>\n");
      fwrite($output_handle, "<geo.long>-93.999705</geo.long>\n");
      fwrite($output_handle, "</extension>\n");
      fwrite($output_handle, "<note type=\"geographic\">Google Maps</note>\n");

      fwrite($output_handle, "</subject>\n");

      if ($data_two[21] !== "") {
//Process field 21 = Collection Name
      fwrite($output_handle, "<relatedItem type=\"host\">\n");
      fwrite($output_handle, "<titleInfo>\n");
      fwrite($output_handle,  "<title>" . trim($data_two[21], ";") . "</title>\n");
      fwrite($output_handle, "</titleInfo>\n");
      fwrite($output_handle, "</relatedItem>\n");
      }

//Process field 28 = Scanning Center; decided to make them all the same
      fwrite($output_handle, "<relatedItem type=\"affiliation\">\n");
      fwrite($output_handle, "<name type=\"fiscal\">\n");
      fwrite($output_handle, "<namePart>Grant provided to the Contributing Institution from the Arts and Cultural Heritage Fund through the vote of Minnesotans on the Nov. 4, 2008, administered by the Minnesota Historical Society.</namePart>\n");
      fwrite($output_handle, "<role>\n");
      fwrite($output_handle, "<roleTerm type=\"text\">Fiscal Sponsor</roleTerm>\n");
      fwrite($output_handle, "</role>\n");
      fwrite($output_handle, "</name>\n");
      fwrite($output_handle, "<name type=\"scan_center\">\n");
      fwrite($output_handle, "<namePart>Northern Micrographics, 2004 Kramer Street, La Crosse, WI 54603</namePart>\n");
      fwrite($output_handle, "<role>\n");
      fwrite($output_handle, "<roleTerm type=\"text\">Scanning Center</roleTerm>\n");
      fwrite($output_handle, "</role>\n");
      fwrite($output_handle, "</name>\n");
      fwrite($output_handle, "</relatedItem>\n");

//Process copyright fields; decided to use same rights management statement for all
      fwrite($output_handle, "<accessCondition>\n");
//      fwrite($output_handle, "<copyright copyright.status="unknown" publication.status="unpublished">\n");
      fwrite($output_handle, "<copyright>\n");
      fwrite($output_handle, "<rights.holder>\n");
      fwrite($output_handle, "<name>Original material located at Minnesota State University, Mankato University Archives</name>\n");
      fwrite($output_handle, "</rights.holder>\n");
      fwrite($output_handle, "<general.note>Use of copyrighted material is governed by U.S. and international copyright laws. Because the resources in this collection cover a wide range of dates, some materials are in the public domain while others are still under copyright protection.  We encourage the fair use of these materials.  Visit our website for additional details. http://lib.mnsu.edu/archives/rights/intro.html</general.note>\n");
      fwrite($output_handle, "</copyright>\n");
      fwrite($output_handle, "</accessCondition>\n");

//Process field 38 = Master File Hardware, field 39 = Master File Software, and field 40 = Master File System
      fwrite($output_handle, "<extension type=\"system\">\n");
      fwrite($output_handle, "<hardware>" . $data_two[36] . "</hardware>\n");
      fwrite($output_handle, "<software>" . $data_two[37] . "</software>\n");
      fwrite($output_handle, "<operatingSystem>" . $data_two[38] . "</operatingSystem>\n");
      fwrite($output_handle, "</extension>\n");

// Write last line to file
   fwrite($output_handle, "</mods>\n");
   fclose($output_handle);

// Delete original .txt file
   unlink($file);
fclose($handle);
}

$log_file = "/home/linda/islandora/msutest/log-ingest1.txt"; //create log file
$log_handle = fopen($log_file, "w");
if ($log_handle === FALSE) {
   echo "log handle is false; couldn't open log file";
   }

$directory = "/home/linda/islandora/msutest/Student";
$issue_directories = array_diff(scandir($directory), array('..', '.'));
foreach ($issue_directories as $key => $issue_value) {
  $count = 1;
  $issue_directory = $directory . "/" . $issue_value;

  createMODSFile($issue_directory, $issue_value, $log_handle);

  $directory_to_scan = $issue_directory . "/scans";
  $scanned_directory = array_diff(scandir($directory_to_scan), array('..', '.'));
  foreach ($scanned_directory as $key => $value) {
    $subdirectory = $issue_directory . "/" . $count;
    mkdir($subdirectory);
    $copy_from = $issue_directory . "/scans/". $value;
    $copy_to = $subdirectory . "/OBJ.tiff";
    copy($copy_from, $copy_to);

// Delete original .tiff file
    unlink($copy_from);
    $count++;
  }

// Delete scans directory
  rmdir($issue_directory . "/scans");
}
echo "Processing complete\n";

fwrite($log_handle, "Processing complete\n");
fclose($log_handle);

?>
