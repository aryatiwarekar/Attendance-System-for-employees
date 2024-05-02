// Enter Spreadsheet ID here
var SS = SpreadsheetApp.openById('google sheet ID');
var timezone = "Asia/Kolkata";
var hours = 0;
var str = '0';

function doPost(e) {
  var parsedData;
  var result = {};

  try {
    parsedData = JSON.parse(e.postData.contents);
  }
  catch (f) {
    return ContentService.createTextOutput("Error in parsing request body: " + f.message);
  }
  // parsedData = JSON.parse('{"command": "insert_row", "sheet_name":"January 2024", "values": "1,M"}');

  if (parsedData !== undefined) {
    var flag = parsedData.format;
    if (flag === undefined) {
      flag = 0;
    }
    var sheet = SS.getSheetByName(parsedData.sheet_name); //SS.getSheetByName(parsedData.sheet_name);
    // sheet name to publish data to is specified in Arduino code
    if (sheet == null) {
      var Curr_Year = Utilities.formatDate(new Date(), timezone, "MMMM yyyy");
      var temp = SS.getSheetByName("template");
      var sheet = SS.insertSheet(parsedData.sheet_name);
      var tempdata = temp.getDataRange().getDisplayValues();
      // console.log(tempdata.length);
      // console.log(tempdata[0].length);
      for (var i = 0; i < tempdata.length; i++) {
        for (var j = 0; j < tempdata[0].length; j++) {
          var r = sheet.getRange(i + 1, j + 1).setValue(tempdata[i][j]);
        }
      }
    }
    var dataArr = parsedData.values.split(","); // creates an array of the values to publish 
    var value0 = dataArr[0];  //Student ID
    var value1 = dataArr[1]; //Gender

    var Curr_Date = Utilities.formatDate(new Date(), timezone, "dd"); // gets the current date
    var Curr_Time = Utilities.formatDate(new Date(), timezone, "hh:mm:ss a"); // gets the current time

    //  var Curr_Time = new Date("Sat Dec 09 2023 20:00:00 GMT+0521 (India Standard Time");
    //  Curr_Time = Utilities.formatDate(Curr_Time, timezone, "hh:mm:ss a")

    //var Curr_Date = new Date(new Date().setHours(new Date().getHours() + hours));
    //var Curr_Time = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss');
    // Curr_Date = "4";

    var data = sheet.getDataRange().getValues();
    for (var i = 1; i < data.length; i = i + 2) {
      if (data[i][0] == value0) {
        var row_number = i + 1;
        for (var j = 3; j < data[0].length; j++) {
          var col_number = j + 1;
          if (j - 2 == Curr_Date) {
            if (row_number > 0) {
              if (sheet.getRange(row_number, col_number).getValue() !== "") {
                if (sheet.getRange(row_number + 1, col_number).getValue() == "") {
                  sheet.getRange(row_number + 1, col_number).setValue(Curr_Time);
                  var otdata = sheet.getRange(row_number, 4, 2, 31).getValues();
                  var regtime = new Array(31).fill(0);
                  var overtime = 0;
                  for (var j = 0; j < 31; j++) {
                    if (otdata[0][j] != "" && otdata[1][j] != "") {
                      regtime[j] = ((otdata[1][j].getTime() - otdata[0][j].getTime()) / 3600000).toFixed(2);
                      if (regtime[j] >= 9) {
                        if(value1 == "M"){
                          overtime += regtime[j] - 9;
                        }
                        else{
                          overtime += regtime[j] - 8;
                        }
                        
                        if (overtime >= 0) {
                          console.log(overtime.toFixed(2));
                        }
                      }
                    }
                  }
                  var totalpay = overtime * (sheet.getRange(row_number,36).getValue());
                  sheet.getRange(row_number,35).setValue(overtime);
                  sheet.getRange(row_number,37).setValue(totalpay);
                  str = 2; // string to return back to Arduino serial console
                  return ContentService.createTextOutput(str);
                }
              }
            }
            if (sheet.getRange(row_number, col_number).getValue() == "") {
              sheet.getRange(row_number, col_number).setValue(Curr_Time);
              str = '1';
              return ContentService.createTextOutput(str);
            }
            else {
              str = '3';
              return ContentService.createTextOutput(str);
            }
          }
        }
      }
    }
  }
  return ContentService.createTextOutput("Something went wrong");
}
