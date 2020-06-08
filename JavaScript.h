// jsfiddle.net
const char JAVASCRIPT[] PROGMEM = R"=====(
//############ DO NOT CHANGE BELOW ###################
// alle GPIOs des ESP8266
const gpio = [  {port: 16, name:'D0'},
                {port: 5,  name:'D1/SDA'},
                {port: 4,  name:'D2/SCL'},
                {port: 0,  name:'D3'},
                {port: 2,  name:'D4'},
                {port: 14, name:'D5'},
                {port: 12, name:'D6'},
                {port: 13, name:'D7'},
                {port: 15, name:'D8'},
                {port: 1,  name:'RX'},
                {port: 3,  name:'TX'}
             ];

window.addEventListener('load', init, false);
function init() {
  SetAvailablePorts();
}

function SetAvailablePorts() {
  var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll('input[type=number][id^=GpioPin]');
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = createPortSelectionList(objects[j].id, objects[j].name, objects[j].value);
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }
}

function createPortSelectionList(id, name, value) {
  _select = document.createElement('select');
  _select.id = id;
  _select.name = name;
  for ( i = 0; i < gpio.length; i += 1 ) {
    // alle GPIO Pins in die Liste
    _option = document.createElement( 'option' );
    _option.value = gpio[i].port; 
    if(gpio_disabled.indexOf(gpio[i].port)>=0) {_option.disabled = true;}
    if(value == (gpio[i].port)) { _option.selected = true;}
    _option.text  = gpio[i].name;
    _select.add( _option ); 
  }
  return _select;
}

function delrow(btn) {
  table = btn.parentNode.parentNode.parentNode;
  if (btn.parentNode.parentNode.rowIndex != 1) {
    // erste Zeile ist das Template, darf nicht entfernt werden
    table.removeChild(btn.parentNode.parentNode);
    validate_identifiers(table.id);
  }
}

function addrow(tableID) { 
  _table = document.getElementById(tableID);
  _table.rows[1].style.display = '';
  new_row = _table.rows[1].cloneNode(true);
  num = _table.rows.length;
  new_row.cells[0].innerHTML = num; 
  objects = new_row.querySelectorAll('label, input, select, div, td');
  for( j=0; j< objects.length; j++) {
    if (objects[j].name) {objects[j].name = objects[j].name.replace(/(\d+)/, num);}
    if (objects[j].id) {objects[j].id = objects[j].id.replace(/(\d+)/, num);}
    if (objects[j].htmlFor) {objects[j].htmlFor = objects[j].htmlFor.replace(/(\d+)/, num);}
  }
  _table.appendChild(new_row);
  validate_identifiers(tableID); // eigentlich obsolete
}

function validate_identifiers(tableID) {
  table = document.getElementById(tableID); 
  for( i=1; i< table.rows.length; i++) { 
    row = table.rows[i];
    row.cells[0].innerHTML = i; 
    objects = row.querySelectorAll('label, input, select, div, td');
    for( j=0; j< objects.length; j++) {
      if (objects[j].name) {objects[j].name = objects[j].name.replace(/(\d+)/, i-1);}
      if (objects[j].id) {objects[j].id = objects[j].id.replace(/(\d+)/, i-1);}
      if (objects[j].htmlFor) {objects[j].htmlFor = objects[j].htmlFor.replace(/(\d+)/, i-1);}
    }
  }
}

function ShowError(t){
  if(t) { t += '<br>Breche Speichervorgang ab. Es wurde nichts gespeichert!' }
  document.getElementById('ErrorText').innerHTML = t;
}

function onSubmit(DataForm, SubmitForm){
  // erstelle json String
  var formData = {};
  ShowError('');
  
  var elems = document.getElementById(DataForm).elements; 
  for(var i = 0; i < elems.length; i++){ 
    if(elems[i].name && elems[i].value) {
      if (elems[i].style.display == 'none') {continue;}
      if (elems[i].parentNode.tagName == 'DIV' && elems[i].parentNode.style.display == 'none') {continue;}
      if (elems[i].parentNode.parentNode.tagName == 'TR' && elems[i].parentNode.parentNode.style.display == 'none') {continue;}
      
      if (elems[i].type == "checkbox") {
        formData[elems[i].name] = (elems[i].checked==true?1:0);
      } else if (elems[i].id.match(/^GpioPin.*/) || elems[i].type == "number") {
        formData[elems[i].name] = parseInt(elems[i].value);
      } else if (elems[i].type == "radio") {
        if (elems[i].checked==true) {formData[elems[i].name] = elems[i].value;}
      } else {
        formData[elems[i].name] = elems[i].value;
      }
    }
  } 
  formData["count"] = document.getElementById(DataForm).getElementsByClassName('editorDemoTable')[0].rows.length -1;
  json = document.getElementById(SubmitForm).querySelectorAll("input[name='json']");
  json[0].value = JSON.stringify(formData);
  
  return true;
}

)=====";
