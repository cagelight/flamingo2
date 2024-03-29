#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include "common.hpp"
#include "imgview.hpp"

enum class SlideshowDirection {Forward, Backward, Random};
struct Options {
	// Slideshow
	std::chrono::milliseconds slideshowInterval {1000};
	SlideshowDirection slideshowDir {SlideshowDirection::Random};
	ImageView::ZKEEP viewKeep {ImageView::KEEP_FIT_FORCE};
	
	// PixelScript
	bool use_ps;
	
	// ViewSave
	bool UUIDAutoSave;
	QString UUIDAutoSaveDir;
	
	void writeSetttings(QSettings &) const;
	void readSettings(QSettings const &);
};

class OptionsWindow : public QWidget {
	Q_OBJECT
signals:
	void applied();
public:
	OptionsWindow(Options opt, QWidget * parent = 0);
	virtual ~OptionsWindow();
	Options const & getOptions() const;
protected:
	Options options;
	virtual void showEvent(QShowEvent *);
	virtual void keyPressEvent(QKeyEvent *);
private:
	QGridLayout * overlayout = nullptr;
	QTabWidget * tabs = nullptr;
	QPushButton * bCancel = nullptr;
	QPushButton * bApply = nullptr;
	QPushButton * bOk = nullptr;

	// Slideshow
	int slideshowTabIndex = 0;
	QWidget * slideshowTabWidget = nullptr;
	QSpinBox * slideshowIntervalSpinbox = nullptr;
	QComboBox * slideshowZoomCBox = nullptr;
	
	// PixelScript
	int psTabIndex = 0;
	QWidget * psTabWidget = nullptr;
	QCheckBox * psCheckbox = nullptr;
	
	// ViewSave
	int vsTabIndex = 0;
	QWidget * vsTabWidget = nullptr;
	QLineEdit * vsUUIDAutoSaveDirLE = nullptr;
	QCheckBox * vsUUIDAutoSaveCBox = nullptr;
private slots:
	void internalApply();
};

#endif //OPTIONS_HPP
