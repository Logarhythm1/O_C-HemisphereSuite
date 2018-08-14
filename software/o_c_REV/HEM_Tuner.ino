#include "src/drivers/FreqMeasure/OC_FreqMeasure.h"
static constexpr double HEM_TUNER_AaboveMidCtoC0 = 0.03716272234383494188492;

class Tuner : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Tuner";
    }

    void Start() {
        A4_Hz = 440;
        FreqMeasure.begin();
    }

    void Controller() {
        if (hemisphere == 1 && FreqMeasure.available()) {
            // average several readings together
            freq_sum_ = freq_sum_ + FreqMeasure.read();
            freq_count_ = freq_count_ + 1;

            if (milliseconds_since_last_freq_ > 250) {
                frequency_ = FreqMeasure.countToFrequency(freq_sum_ / freq_count_);
                freq_sum_ = 0;
                freq_count_ = 0;
                milliseconds_since_last_freq_ = 0;
            }
        } else if (milliseconds_since_last_freq_ > 100000) {
            frequency_ = 0.0f;
        }
    }

    void View() {
        gfxHeader(applet_name());
        if (hemisphere == 1) DrawTuner();
        else DrawWarning();
    }

    void ScreensaverView() {
        if (hemisphere == 1) DrawTuner();
        else DrawWarning();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
        A4_Hz = constrain(A4_Hz + direction, 400, 500);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, A4_Hz);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        A4_Hz = Unpack(data, PackLocation {0,16});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "2=Input";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "";
        help[HEMISPHERE_HELP_ENCODER]  = "A4 Hz";
        //                               "------------------" <-- Size Guide
    }
    
private:
    // Port from References
    double freq_sum_;
    uint32_t freq_count_;
    float frequency_ ;
    elapsedMillis milliseconds_since_last_freq_;
    int A4_Hz; // Tuning reference

    void DrawTuner() {
        float frequency_ = get_frequency() ;
        float c0_freq_ = get_C0_freq() ;

        int32_t deviation = round(12000.0 * log2f(frequency_ / c0_freq_)) + 500;
        int8_t octave = deviation / 12000;
        int8_t note = (deviation - (octave * 12000)) / 1000;
        note = constrain(note, 0, 12);
        int32_t residual = ((deviation - ((octave - 1) * 12000)) % 1000) - 500;

        if (frequency_ > 0.0) {
            gfxPrint(20, 30, OC::Strings::note_names[note]);
            gfxPrint(" ");
            gfxPrint(octave);
            if (residual < 10 && residual > -10) gfxInvert(1, 28, 62, 11);
            else {
                // Don't show the residual if the tuning is perfect
                if (residual >= 0) {
                    gfxPrint(22, 45, "+");
                    gfxPrint(residual / 10);
                } else {
                    gfxPrint(22, 45, residual / 10);
                }
                gfxPrint("c");
            }

            // Draw some sort of indicator
            if (residual > 10) {
                uint8_t n = residual / 100;
                for (uint8_t i = 0; i < (n + 1); i++)
                {
                    gfxPrint(48 + (i * 2), 45, "<");
                }
            } else if (residual < -10) {
                uint8_t n = -(residual / 100);
                for (uint8_t i = 0; i < (n + 1); i++)
                {
                    gfxPrint(10 - (i * 2), 45, ">");
                }

            }
        }

        gfxPrint(1, 15, "A4= ");
        gfxPrint(A4_Hz);
        gfxPrint(" Hz");
        gfxCursor(25, 23, 36);
    }
    
    void DrawWarning() {
        gfxPrint(1, 15, "Tuner goes");
        gfxPrint(1, 25, "in right");
        gfxPrint(1, 35, "hemisphere");
        gfxPrint(1, 45, "       -->");
    }

    float get_frequency() {return frequency_;}
    
    float get_C0_freq() {
        return(static_cast<float>(A4_Hz * HEM_TUNER_AaboveMidCtoC0));
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Tuner,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Tuner Tuner_instance[2];

void Tuner_Start(int hemisphere) {
    Tuner_instance[hemisphere].BaseStart(hemisphere);
}

void Tuner_Controller(int hemisphere, bool forwarding) {
    Tuner_instance[hemisphere].BaseController(forwarding);
}

void Tuner_View(int hemisphere) {
    Tuner_instance[hemisphere].BaseView();
}

void Tuner_Screensaver(int hemisphere) {
    Tuner_instance[hemisphere].BaseScreensaverView();
}

void Tuner_OnButtonPress(int hemisphere) {
    Tuner_instance[hemisphere].OnButtonPress();
}

void Tuner_OnEncoderMove(int hemisphere, int direction) {
    Tuner_instance[hemisphere].OnEncoderMove(direction);
}

void Tuner_ToggleHelpScreen(int hemisphere) {
    Tuner_instance[hemisphere].HelpScreen();
}

uint32_t Tuner_OnDataRequest(int hemisphere) {
    return Tuner_instance[hemisphere].OnDataRequest();
}

void Tuner_OnDataReceive(int hemisphere, uint32_t data) {
    Tuner_instance[hemisphere].OnDataReceive(data);
}
