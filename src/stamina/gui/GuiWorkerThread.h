#ifndef STAMINA_GUI_GUIWORKERTHREAD
#define STAMINA_GUI_GUIWORKERTHREAD

#include <QThread>

#include <KMessageBox>

#include <Stamina.h>

namespace stamina {
	namespace gui {
		class GuiWorkerThread : public QThread {
			Q_OBJECT
		public:
			/**
			* Runs STAMINA from the given information
			* */
			GuiWorkerThread(QObject * parent) : QThread(parent) { /* Intentionally left empty */ }
			virtual ~GuiWorkerThread() { /* Intentionally left empty */ }
			void run() override {
				successful = false;
				try {
					s->run(mustRebuildModel);
					successful = true;
					StaminaMessages::info("Finished running, should emit finished() now");
					// this->quit();
				}
				catch (std::string & e) {
					std::string msg = std::string("Error got while running STAMINA: ") + e;
					KMessageBox::error(nullptr, QString::fromStdString(msg));
					this->exit(1);
				}
				catch (storm::exceptions::BaseException & e) {
					std::string msg = std::string("Error in Storm: ") + e.what();
					KMessageBox::error(nullptr, QString::fromStdString(msg));
					this->exit(1);
				}
				// emit finished(QThread::QPrivateSignal());
			}
			bool wasSuccessful() { return successful; }
			/**
			* Sets the STAMINA instance to be run from
			* */
			void setStaminaInstance(Stamina * s) { this->s = s; }
			/**
			* Sets whether or not to rebuild the model
			* */
			void setMustRebuildModel(bool mustRebuildModel) { this->mustRebuildModel = mustRebuildModel; }
		private:
			Stamina * s;
			bool mustRebuildModel;
			bool successful;
		};

	}
}

#endif // STAMINA_GUI_GUIWORKERTHREAD
