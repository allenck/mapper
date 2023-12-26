<?php
header('Access-Control-Allow-Origin: *');
$format = $_GET['format'];
// show list of all tilesets along with links
{
	if($format != 'csv')
  {
   print '<h2>MBTiles PHP proxy</h2>';
  }
	if( $handle = opendir('.') ) 
  {
		$found = false;
		while( ($file = readdir($handle)) !== false ) 
    {
			if( preg_match('/^[\w\d_-]+\.mbtiles$/', $file) && is_file($file) ) 
      {
				try 
        {
					$db = new PDO('sqlite:'.$file);
					$params = readparams($db);
					$zooms = readzooms($db);
					$db = null;
          if($format == 'csv')
          {
           print $params['name'];
           print '|';
           print $params['description'];
           print '|';
           $minZoom = 99;
           $maxZoom = 0;
           foreach($zooms as $zoom)
           {
            if(floatval($zoom) < $minZoom)
             $minZoom = $zoom;
            if(floatval($zoom) > $maxZoom)
             $maxZoom = $zoom;
           }
           print $minZoom;
           print '|';
           print $maxZoom;
           print "|65|";
           print $params['bounds'];
           print '|';
           print $params['center'];
           print '<br>';
          }
          else
          { 
					print '<h3>'.htmlspecialchars($params['name']).'</h3>';
					if( isset($params['description']) )
						print '<p>'.htmlspecialchars($params['description']).'</p>';
					print '<p>Type: '.$params['type'].', format: '.$params['format'].', version: '.$params['version'].'</p>';
					if( isset($params['bounds']) )
						print '<p>Bounds: '.str_replace(',', ', ',$params['bounds']).'</p>';
					print '<p>Zoom levels: '.implode(', ', $zooms).'</p>';
					print '<p>OpenLayers: <tt>new OpenLayers.Layer.OSM("'.htmlspecialchars($params['name']).'", "'.getbaseurl().preg_replace('/\.mbtiles/','',$file).'/${z}/${x}/${y}", {numZoomLevels: '.(end($zooms)+1).', isBaseLayer: '.($params['type']=='baselayer'?'true':'false').'});</tt></p>';
					print '<p>TMS: <tt>http://'.$_SERVER['HTTP_HOST'].preg_replace('/\/[^\/]+$/','/',$_SERVER['REQUEST_URI']).'1.0.0/'.preg_replace('/\.mbtiles/','',$file).'</tt></p>';
          }
				} catch( PDOException $e ) {}
			}
		}
  
  }else {
		print 'Error opening script directory.';
	}
}

function getbaseurl() {
	return 'http://'.$_SERVER['HTTP_HOST'].preg_replace('/\/(1.0.0\/)?[^\/]*$/','/',$_SERVER['REQUEST_URI']);
}

function readparams( $db ) {
	$params = array();
	$result = $db->query('select name, value from metadata');
	while ($row = $result->fetch(PDO::FETCH_ASSOC)) {
		$params[$row['name']] = $row['value'];
	}
	return $params;
}
function readzooms( $db ) {
	$zooms = array();
	$result = $db->query('select zoom_level from tiles group by zoom_level order by zoom_level');
	while ($zoom = $result->fetchColumn()) {
		$zooms[] = $zoom;
	}
	return $zooms;
}
?>
