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
        new Characteristic::ConfiguredName("阈值设置");
        On->setVal(threshold_int > 0);
        Dynamic_threshold = new Characteristic::Brightness(threshold_int);
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
    homeSpan.begin(Category::Bridges, "响指传感器");
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("响指传感器");
    new ThresholdSetting(threshold);
    SpanService *service = new Service::StatelessProgrammableSwitch(); // create a new Stateless Programmable Switch Service
    this->switchEvent = new Characteristic::ProgrammableSwitchEvent();
    new Characteristic::ConfiguredName("响指传感器");
    homeSpan.setApTimeout(1000);
    homeSpan.enableAutoStartAP();
    homeSpan.autoPoll();
}
