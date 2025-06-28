<?php
$zoom = $_GET['z'];
$column = $_GET['x'];
$row = $_GET['y'];
$db = $_GET['db'];
$files = "";

  if($db == "" || $db == null)
  {
   $files = scandir(".");
   #print_r($files);
   foreach($files as $file)
   {
    if(endswith($file, ".mbtiles"))
    {
     $db = substr($file, 0, strlen($file)-8);
     #echo "Value: $file  Db: $db <br/>\n";
    
     try
     {
      // Open the database
      $conn = new PDO("sqlite:$file");

      $sql = "select name, value from metadata";
      #echo "sql: $sql <br/>\n";
      $q = $conn->prepare($sql);
      $rows = $q->execute();
      #echo "$db : rows $rows <br/>\n";

      $q->bindColumn(1, $metaname);
      $q->bindColumn(2, $metavalue);

      while($q->fetch())
      {
       if($metaname == "name")
        $name = $metavalue;
       if($metaname == "description")
        $description = $metavalue;
       if($metaname == "bounds")
        $bounds = $metavalue;
       if($metaName == "center")
        $center = $metavalue;
       if($metaname == "minzoom")
        $minZoom = $metavalue;
       if($metaname == "maxzoom")
        $maxZoom = $metavalue;
      }
      reset($conn);
      echo "$name|$description|$minZoom|$maxZoom|65|$bounds|$center<br/>\n";
     }
     catch(PDOException $e)
     {
      print 'Exception : '.$e->getMessage();
      echo "<br/>\n";
     }
    }
   }
   exit;
  }
  try
  {
    // Open the database
    $conn = new PDO("sqlite:$db");

    // Query the tiles view and echo out the returned image
	$sql = "SELECT * FROM tiles WHERE zoom_level = $zoom AND tile_column = $column AND tile_row = $row";
	$q = $conn->prepare($sql);
	$q->execute();

	$q->bindColumn(1, $zoom_level);
	$q->bindColumn(2, $tile_column);
	$q->bindColumn(3, $tile_row);
	$q->bindColumn(4, $tile_data, PDO::PARAM_LOB);

	while($q->fetch())
	{
	header("Content-Type: image/png");
	echo $tile_data;
	}
  }
  catch(PDOException $e)
  {
    print 'Exception : '.$e->getMessage();
  }

function endsWith($haystack, $needle)
{
    $length = strlen($needle);
    if ($length == 0) {
        return true;
    }

    return (substr($haystack, -$length) === $needle);
}
?>
