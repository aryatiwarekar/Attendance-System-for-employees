function onEditTrigger(e) {
  var sheet = e.source.getActiveSheet();
  console.log(sheet.getName());
  var data = sheet.getDataRange().getValues();
  var regtime = new Array(31).fill(0);
  var overtime = 0;
  console.log(data[20].length);
  for (var i = 1; i < data.length; i = i + 2) {
    var otdata = sheet.getRange(i + 1, 4, 2, 31).getValues();
    for (var j = 0; j < otdata[0].length; j++) {
      if (otdata[0][j] != "" && otdata[1][j] != "") {
        regtime[j] = ((otdata[1][j].getTime() - otdata[0][j].getTime()) / 3600000).toFixed(2);
        if (regtime[j] >= 9) {
          if (data[i][37] == "M") {
            overtime += regtime[j] - 9;
          }
          else {
            overtime += regtime[j] - 8;
          }
          if (overtime >= 0) {
            console.log(overtime.toFixed(2));
          }
        }
      }
    }
    var totalpay = overtime * (sheet.getRange(i + 1, 36).getValue());
    sheet.getRange(i + 1, 35).setValue(overtime);
    sheet.getRange(i + 1, 37).setValue(totalpay);
    regtime.fill(0);
    overtime = 0;
  }

}
