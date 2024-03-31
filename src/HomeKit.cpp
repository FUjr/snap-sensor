#include "HomeKit"
HomeKit::HomeKit(void)
{
    homeSpan.setStatusPixel(38, 100, 100, 100);
    homeSpan.setControlPin(13);
    homeSpan.begin(Category::ProgrammableSwitches, "响指传感器");
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("响指传感器");
    SpanService *service = new Service::StatelessProgrammableSwitch(); // create a new Stateless Programmable Switch Service
    this->switchEvent = new Characteristic::ProgrammableSwitchEvent();
    homeSpan.setApTimeout(1000);
    homeSpan.enableAutoStartAP();
    homeSpan.autoPoll();
}
