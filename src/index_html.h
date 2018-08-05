#ifndef INDEX_HTML
  #define INDEX_HTML


#include "html_handlers.h"




String index_html(){
  // Renvoi une chaine contenant le code HTML de la page - Return a string containing the HTML code of the page
  String page = "<html charset=UTF-8><head><meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1'/>";
  page += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script><script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>";
  page += "<title>Water Pump Control</title></head><body>";
  page += "<div class='container-fluid'>";
  page +=   "<div class='row'>";
  page +=     "<div class='col-md-12'>";
  page +=       "<h1>Water Pump Control</h1>";
  page +=       "<h3>Amperage/Pressure Monotoring</h3>";
  page +=       "<ul class='nav nav-pills'>";
  page +=         "<li class='active'>";
  page +=           "<a href='#'> <span class='badge pull-right' id='AmpValue'>";
  page +=           amps;
  page +=           "</span> Amp</a>";
  page +=         "<li class='active'>";
  page +=           "<a href='#'> <span class='badge pull-right' id='BarValue'>";
  page +=           bars;
  page +=           "</span> Bar</a>";
  page +=       "</ul>";
  page +=       "<h3>Control</h3>";
  page +=       "<div class='row'>";
  page +=         "<div class='col-md-1'><h4 class ='text-left'>D5 ";
  page +=           "<span class='badge'>";
  page +=           etatGpio[0];
  page +=         "</span></h4></div>";
  page +=         "<div class='col-md-1'><form action='/' method='POST'><button type='button submit' name='D5' value='1' class='btn btn-success btn-lg'>ON</button></form></div>";
  page +=         "<div class='col-md-1'><form action='/' method='POST'><button type='button submit' name='D5' value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
  page +=       "</div>";
  page +=       "<h3>Upload Firmware</h3>";
  page +=       "<div class='row'>";
  page +=         "<div class='col-md-1'><h4 class ='text-left'>D5 ";
  page +=           "<span class='badge'>";
  page +=           etatGpio[0];
  page +=         "</span></h4></div>";
  page +=         "<div class='col-md-1'><form action='/' method='POST'><button type='button submit' name='D5' value='1' class='btn btn-success btn-lg'>ON</button></form></div>";
  page +=         "<div class='col-md-1'><form action='/' method='POST'><button type='button submit' name='D5' value='0' class='btn btn-danger btn-lg'>OFF</button></form></div>";
  page +=       "</div>";
  page +=   "<div class='row'>";
  page +=       "<div class='col-md-3'>";
  page +=         "<p>Made by: <a href='mailto:pedronsg@gmail.com'>Pedro Gomes</a></p>";
  page +=       "</div>";
  page +=   "</div></div></div></div>";

  page +=   "<script>";
  page +=   "function sendData(led) {";
  page +=   "var xhttp = new XMLHttpRequest();";
  page +=   "xhttp.onreadystatechange = function() {";
  page +=   "if (this.readyState == 4 && this.status == 200) {";
  page +=   "document.getElementById('LEDState').innerHTML =";
  page +=   "this.responseText;";
  page +=   "}";
  page +=   "};";
  page +=   "xhttp.open('GET', 'setLED?LEDstate='+led, true);";
  page +=   "xhttp.send();";
  page +=   "}";

  page +=   "setInterval(function() {";
  page +=   "  getData();";
  page +=   "}, 1000);"; //2000mSeconds update rate

  page +=   "function getData() {";
  page +=     "var xhttp = new XMLHttpRequest();";
  page +=     "xhttp.onreadystatechange = function() {";
  page +=       "if (this.readyState == 4 && this.status == 200) {";
  page +=         "var res = this.responseText.split('#');";
  page +=         "document.getElementById('AmpValue').innerHTML = res[0];";
  page +=         "document.getElementById('BarValue').innerHTML = res[1];";
  page +=       "}";
  page +=     "};";
  page +=     "xhttp.open('GET', 'readData', true);";
  page +=     "xhttp.send();";
  page +=   "}";


  page +=   "</script>";

  page += "</body></html>";
  return page;
}

#endif
