<?php
$target_dir = "./uploadstore/";
if (!file_exists($target_dir)) {
    mkdir($target_dir, 0777, true);
}

$target_file = $target_dir . basename($_FILES["file"]["name"]);

if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_file)) {
    echo "File '". basename($_FILES["file"]["name"]). "' uploaded successfully to $target_file.";
} else {
    echo "Error uploading file.";
}
?>
