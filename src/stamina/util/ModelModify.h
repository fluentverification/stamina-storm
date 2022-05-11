#ifndef MODELMODIFY_H
#define MODELMODIFY_H
namespace stamina {
	namespace util {
		class ModelModify {
		public:
			/**
			 * Constructor for the model file modifier
			 *
			 * @param originalModel The path to the original model file
			 * @param originalProperties The path to the original properties file
			 * @param saveModifiedModel Whether or not to save (or rather, not delete) the modified model file
			 * @param saveModifiedProperties Whether or not to save (or rather, not delete) the
			 *     modified properties list
			 * @param modifiedModel The file where we store the modified model
			 * @param modifiedProperties The file where we store the modified properties
			 * **/
			ModelModify(
				std::string originalModel
				, std::string originalProperties
				, bool saveModifiedModel = true
				, bool saveModifiedProperties = true
				, std::string modifiedModel = "modelified-model-stamina.prism"
				, std::string modifiedProperties = "modified-properties-stamina.csl"
			);
			/**
			 * The destructor for the model modifier
			 * **/
			~ModelModify();
			/**
			 * Creates the modified model
			 * **/
			std::shared_ptr<storm::prism::Program> createModifiedModel();
			/**
			 * Creates the modified Properties
			 * **/
			std::shared_ptr<std::vector<storm::jani::Property>> createModifiedProperties();
		private:
			std::string originalModel;
			std::string modifiedModel;
			std::string originalProperties;
			std::string modifiedProperties;
			bool saveModifiedModel;
			bool saveModifiedProperties;
		};
	} // namespace util
} // namespace stamina

#endif // MODEL_MODIFY_H
