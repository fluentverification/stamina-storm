/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

#ifndef STAMINA_UTIL_MODELMODIFY_H
#define STAMINA_UTIL_MODELMODIFY_H

#include <vector>

#include <storm/api/storm.h>
#include <storm-parsers/api/storm-parsers.h>
#include <storm-parsers/parser/PrismParser.h>
#include <storm/storage/prism/Program.h>
#include <storm/storage/jani/Property.h>

#include "storm/storage/jani/Property.h"
#include "storm/storage/prism/Program.h"

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
				std::string model
				, std::string properties
			);
			/**
			 * The destructor for the model modifier
			 * **/
			~ModelModify();
			/**
			 * Reads the model. I don't believe we need to modify it at all
			 * **/
			std::shared_ptr<storm::prism::Program> readModel();
			/**
			 * Creates the modified Properties list. Each element in the list contains
			 *     - The original property
			 *     - The Pmin
			 *     - The Pmax
			 * **/
			std::shared_ptr<std::vector<storm::jani::Property>> createPropertiesList(
				std::shared_ptr<storm::prism::Program> modelFile
			);
			/**
			 * Creates a modified property
			 *
			 * @param isMin whether or not to make it min
			 * @return Modified property
			 * */
			storm::jani::Property modifyProperty(
				storm::jani::Property prop
				, bool isMin
			);
			/**
			 * Allows you to set the model and properties file path
			 * after the model modify has been constructed.
			 *
			 * @param model New model file path
			 * @param properties New Properties file path
			 * */
			void setModelAndProperties(std::string model, std::string properties);
			/**
			 * Getters for model and properties
			 * */
			std::string getModel();
			std::string getProperties();
		private:
			std::string model;
			std::string properties;
			std::shared_ptr<storm::prism::Program> modelFile;
		};
	} // namespace util
} // namespace stamina

#endif // STAMINA_UTIL_MODELMODIFY_H
