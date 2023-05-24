#include "Preferences.h"

namespace stamina {
namespace gui {

const std::string PrefInfo::prefPath = ".xstaminarc";

Preferences::Preferences(QWidget *parent)
	: QDialog(parent)
{
	setupActions();
}

void
Preferences::setupActions() {
	ui.setupUi(this);
}

void
Preferences::show() {

}

void
Preferences::hide() {

}

void 
Preferences::setOptionsFromPreferences() {
	// General options
	
	// ModelBuilding tab Options
	core::Options::kappa = ModelBuilding::kappa;
	core::Options::reduce_kappa = ModelBuilding::rKappa;
	core::Options::prob_win = ModelBuilding::window;
	core::Options::no_prop_refine = !ModelBuilding::earlyTerminationProperty;
	// TODO: maxIterations
	core::Options::max_approx_count = ModelBuilding::maxApproxIterations;
	if (ModelBuilding::exportPerimeterStates && ModelBuilding::perimeterStatesFile == "") {
		// TODO: Error
	}
	else if (ModelBuilding::exportPerimeterStates) {
		core::Options::export_perimeter_states = ModelBuilding::perimeterStatesFile;
	}
	core::Options::method = ModelBuilding::truncationMethod;
	core::Options::threads = ModelBuilding::threads;
}

} // namespace gui
} // namespace stamina
