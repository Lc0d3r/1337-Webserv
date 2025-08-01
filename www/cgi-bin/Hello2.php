#!/usr/bin/php-cgi
<?php
// Print headers required by CGI
echo "Content-Type: text/html\r\n\r\n";

// Output a simple HTML response
echo "<html><body>";
echo "<h1>Hello from PHP CGI!</h1>";

// If the request is POST, show the body
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "<h2>POST Data:</h2>";
    echo "<pre>";
    print_r($_POST);
    echo "</pre>";
}

echo "</body></html>";
?>
