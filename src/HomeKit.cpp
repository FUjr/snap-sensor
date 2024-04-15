#include "HomeKit"

struct ThresholdSetting : Service::LightBulb {
    SpanCharacteristic *Dynamic_threshold;
    SpanCharacteristic *On;
    float* threshold;
    int threshold_int;
    ThresholdSetting(float * threshold_ptr) : Service::LightBulb(){
        threshold = threshold_ptr;
        threshold_int = *threshold * 100;
        On = new Characteristic::On();
        new Characteristic::ConfiguredName("Threshold");
        On->setVal(threshold_int > 0);
        Dynamic_threshold = new Characteristic::Brightness(threshold_int,true);
    }

    boolean update(){
        threshold_int = Dynamic_threshold->getNewVal();
        On->setVal(threshold_int > 0);
        *threshold = threshold_int / 100.0;
        return true;
    }
};

HomeKit::HomeKit(float *threshold)
{   
    this->threshold = threshold;
    homeSpan.setStatusPixel(38, 100, 100, 100);
    homeSpan.setControlPin(13);
    homeSpan.begin(Category::Bridges, "Snap Sensor");
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("Snap Sensor");
    new ThresholdSetting(threshold);
    SpanService *scoreService = new Service::LightSensor();
    this->currentScore = new Characteristic::CurrentAmbientLightLevel(0.0001);
    new Characteristic::ConfiguredName("Score");
    SpanService *service = new Service::StatelessProgrammableSwitch(); // create a new Stateless Programmable Switch Service
    this->switchEvent = new Characteristic::ProgrammableSwitchEvent();
    new Characteristic::ConfiguredName("Snap Trigger");
    homeSpan.setApTimeout(1000);
    homeSpan.enableAutoStartAP();
    homeSpan.autoPoll();
}
