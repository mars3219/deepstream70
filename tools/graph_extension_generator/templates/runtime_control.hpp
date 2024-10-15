/**
 * @brief Helper component to control(set/get) properties of <CLASSNAME>
 * component. This is a connector component. It must be
 * linked to \'<CLASSNAME>\' for which properties are to be controlled and
 * another component which will set/get properties via the helper component
 * APIs.
 */
class <CLASSNAME>PropertyController : public <DS_PREFIX>INvDsPropertyController {
 public:
  <PROPMETHODS>

  /**
   * @brief Set the element component whose properties will be controlled.
    * This method is called by <CLASSNAME> component.
    */
  void set_element(INvDsElement *element) override {
    this->element_ = element->get_element_ptr();
  }

 private:
  GstElement *element_;
};