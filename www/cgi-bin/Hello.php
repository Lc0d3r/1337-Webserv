#!/usr/bin/php-cgi
<?php
// CGI requires output to start with a proper HTTP header
header("Content-Type: text/plain");

echo "<h1>Hello from PHP-CGI!</h1>";
echo "<p>Request Method: " . $_SERVER["REQUEST_METHOD"] . "</p>";
?>