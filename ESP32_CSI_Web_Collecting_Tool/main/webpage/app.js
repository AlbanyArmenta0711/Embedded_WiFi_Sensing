$(document).ready(function () {
    // Hide all relevant sections
    $("#sdSection, #sdOptions, #uartcommserialSection, #csOptions, #spicommserialSection").hide();

    $("#mac label").text("Transmitter MAC address:");

          // Event listener to change options based on user' selection (Receiver or Transmitter)
      $("#op_modes").on("change", function () {
          // Get the selected option value
          let selectedOption = $(this).val();

          // Hide all relevant sections
          $("#sample, #info-mode").hide();

           // Show or hide the relevant section based on the selected option
          if (selectedOption === "1") {
              $("#sample, #info-mode, #message").show();
              // Change label text for Transmitter
              $("#mac label").text("Transmitter MAC address:");
          } else if (selectedOption === "2") {
              $("#message, #sample").hide();
              // Change label text for Receiver
              $("#mac label").text("Receiver MAC address:");
          }
      });


  $("#submit_button").on("click", function (event) {

      event.preventDefault();

      // Perform validation before submission
      if (!validateForm()) {
              return; // Exit if validation fails
          }
      
      // Extract selected options from form
      const opmodes = $("#op_modes").val();
      const macA = $("#macAddress").val();
      const bandW = $("#bandwidth").val();
      const frecuencia=$("#frecuencia").val();
      const communication = $("#comm").val();
      const muestra = $("input[name='muestra']:checked").val();

      // Extract checkbox values
      const macAddressf = $("#macAddressf").prop("checked");
      const csiBuf = $("#csiBuf").prop("checked");
      const rssi = $("#rssi").prop("checked");
      const channelBandwidth = $("#channelBandwidth").prop("checked");
      const noiseFloor = $("#noiseFloor").prop("checked");
      const timestamp = $("#timestamp").prop("checked");
      const antennaNumber = $("#antennaNumber").prop("checked");
      const csiDataLength = $("#csiDataLength").prop("checked");

       // Extract values from Informer Mode section
      const informerMode = $("#informer_mode").val();

      // Extract values from SD section (if displayed)
      const miso = $("#miso").val();
      const mosi = $("#mosi").val();
      const clk = $("#clk").val();
      const cs = $("#cs").val();

      // Extract values from UART serial communication section (if displayed)
      const baudrate = $("#baudrate").val();
      const tx = $("#tx").val();
      const rx = $("#rx").val();

      //Extract values from SPI serial communication section
      const data_0 = $("#data_0").val();
      const data_1 = $("#data_1").val();
      const data_2 = $("#data_2").val();
      const data_3 = $("#data_3").val();
      const data_4 = $("#data_4").val();
      const data_5 = $("#data_5").val();
      const data_6 = $("#data_6").val();
      const data_7 = $("#data_7").val();
      const sclk = $("#sclk").val();
      const cs_spi = $("#cs_spi").val();
      const handshake = $("#handshake").val();

      // Send the data and selected options to the server using an AJAX request
      $.ajax({
          url: "/csiConfiguration.json",
          type: "POST",
          //data: JSON.stringify(data),
          contentType: "application/json",
          headers: {
              "OpModes": opmodes,
              "MacAddress": macA, // Send MAC address as a string
              "Communication": communication,
              "Frequency": frecuencia,
              "Bandwidth": bandW,
              "DataRepresentation": muestra,
              "IsMacAddress": macAddressf,
              "IsCSIBuff": csiBuf,
              "IsRSSI": rssi,
              "IsChannelBandwidth": channelBandwidth,
              "IsNoiseFloor": noiseFloor,
              "IsTimestamp": timestamp,
              "IsAntennaNumber": antennaNumber,
              "IsCSIDataLength": csiDataLength,
               // Add values from SD and COMUNICACION SERIAL sections as needed
              "InformerMode": informerMode,
              "MISO": miso,
              "MOSI": mosi,
              "CLK": clk,
              "CS": cs,
              "Baudrate": baudrate,
              "TX": tx,
              "RX": rx,
              "Data0": data_0,
              "Data1": data_1,
              "Data2": data_2,
              "Data3": data_3, 
              "Data4": data_4,
              "Data5": data_5, 
              "Data6": data_6,
              "Data7": data_7, 
              "SCLK": sclk, 
              "SPICS": cs_spi, 
              "Handshake": handshake
          },
          success: function (response) {
              // Handle the success response
              console.log(response);

              // Hide the form and show the success message
              $("#yourFormId").hide();
              $("#successMessage").show();
          },
          error: function (jqXHR, textStatus, errorThrown) {
              // Handle errors
              console.log(textStatus, errorThrown);
          },
      });

  });

  function validateForm() {
      $(".form-control").removeClass("is-invalid");
      $(".invalid-feedback").hide();
  
      const operationMode = $("#op_modes").val();
      const macA = $("#macAddress").val();
      const bandW = $("#bandwidth").val();
      const frec = $("#frecuencia").val();
      const communication = $("#comm").val();
      const muestra = $("input[name='muestra']:checked").length > 0;
      const informerMode = $("#informer_mode").val();
      const messageStructureChecked = $("#message input[type='checkbox']:checked").length > 0;
      const sdSectionVisible = $("#sdSection").is(":visible");
      const uartcommserialSectionVisible = $("#uartcommserialSection").is(":visible");
  
      if (operationMode == 1) {
          if (!macA || !bandW  || !frec || !communication || !muestra || !informerMode || !messageStructureChecked) {
              $("#macAddress, #bandwidth, #comm, #frecuencia, #sample, #message, #informer_mode")
                  .filter(function () {
                      return !$(this).val();
                  })
                  .addClass("is-invalid")
                  .next(".invalid-feedback")
                  .show();
              if (!messageStructureChecked) {
                  $("#message .invalid-feedback").show();
              }
              if (!muestra) {
                  $("#sample .invalid-feedback").show();
              }
              return false;
          }

          if (informerMode == 2 && (!sdSectionVisible || !$("#miso").val() || !$("#mosi").val() || !$("#clk").val() || !$("#cs").val())) {
              $("#sdSection input")
                  .filter(function () {
                      return !$(this).val();
                  })
                  .addClass("is-invalid")
                  .next(".invalid-feedback")
                  .show();
              return false;
          }
      
          if (informerMode == 3 && (!uartcommserialSectionVisible || !$("#baudrate").val() || !$("#tx").val() || !$("#rx").val())) {
              $("#uartcommserialSection input")
                  .filter(function () {
                      return !$(this).val();
                  })
                  .addClass("is-invalid")
                  .next(".invalid-feedback")
                  .show();
              return false;
          }
      
          if (informerMode == 4){
              if((!sdSectionVisible || !$("#data_0").val() || !$("#data_1").val() || !$("#data_2").val() || !$("#data_3").val() || !$("#data_4").val() || !$("#data_5").val() || !$("#data_6").val() || !$("#data_7").val() || !$("#sclk").val() || !$("#cs_spi").val() || !$("#handshake").val())){
                  $("#spicommserialSection input")
                      .filter(function () {
                          return !$(this).val();
                      })
                      .addClass("is-invalid")
                      .next(".invalid-feedback")
                      .show();
                  return false;
              }
          } 


      } else if (operationMode == 2) {
          if (!frec || !macA) {
              $("#frecuencia, #macAddress")
                  .filter(function () {
                      return !$(this).val();
                  })
                  .addClass("is-invalid")
                  .next(".invalid-feedback")
                  .show();
              return false;
          }
      }
  
  
      return true;
  }
  
     // Event listener for "Return to Form" button
     $("#returnButton").on("click", function () {

       // Reset the form
      //$("#yourFormId")[0].reset();

      // Hide all relevant sections
      $("#sdSection, #sdOptions, #uartcommserialSection, #csOptions").hide();
      
      // Show the form and hide the success message
      $("#yourFormId").show();
      $("#successMessage").hide();
    
  });

  // Event listener for option selection
  $("#informer_mode").on("change", function () {
      // Get the selected option value
      let selectedOption = $(this).val();

      // Hide all relevant sections
      $("#sdSection, #sdOptions, #uartcommserialSection, #spicommserialSection").hide();

      // Show the relevant section based on the selected option
      if (selectedOption === "2") {
          $("#sdSection").show();
          $("#sdOptions").show();
      } else {
          if (selectedOption === "3") {
          $("#uartcommserialSection").show();
          }else{
              if (selectedOption === "4") {
                  $("#spicommserialSection").show();
              }
          }
      }
  });
});



