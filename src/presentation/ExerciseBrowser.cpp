#include "presentation/ExerciseBrowser.h"
#include "adapters/InMemoryExerciseRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>

namespace MusicTrainer {
namespace presentation {

ExerciseBrowser::ExerciseBrowser(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    setupConnections();
    populateExercises();
}

ExerciseBrowser::~ExerciseBrowser() = default;

std::shared_ptr<domain::exercises::Exercise> ExerciseBrowser::getSelectedExercise() const
{
    return m_selectedExercise;
}

void ExerciseBrowser::setExerciseRepository(std::shared_ptr<domain::exercises::ExerciseRepository> repository)
{
    m_repository = std::move(repository);
    populateExercises();
}

void ExerciseBrowser::setupUi()
{
    setWindowTitle(tr("Exercise Browser"));
    resize(800, 600);
    
    auto* mainLayout = new QVBoxLayout(this);
    
    // Create filter section
    auto* filterGroupBox = new QGroupBox(tr("Filters"), this);
    auto* filterLayout = new QHBoxLayout(filterGroupBox);
    
    auto* difficultyLabel = new QLabel(tr("Difficulty:"), this);
    m_difficultyComboBox = new QComboBox(this);
    m_difficultyComboBox->addItem(tr("All Levels"));
    m_difficultyComboBox->addItem(tr("Beginner"));
    m_difficultyComboBox->addItem(tr("Intermediate"));
    m_difficultyComboBox->addItem(tr("Advanced"));
    
    auto* categoryLabel = new QLabel(tr("Category:"), this);
    m_categoryComboBox = new QComboBox(this);
    m_categoryComboBox->addItem(tr("All Categories"));
    m_categoryComboBox->addItem(tr("First Species"));
    m_categoryComboBox->addItem(tr("Second Species"));
    m_categoryComboBox->addItem(tr("Third Species"));
    m_categoryComboBox->addItem(tr("Fourth Species"));
    m_categoryComboBox->addItem(tr("Fifth Species"));
    m_categoryComboBox->addItem(tr("Free Counterpoint"));
    
    filterLayout->addWidget(difficultyLabel);
    filterLayout->addWidget(m_difficultyComboBox);
    filterLayout->addSpacing(20);
    filterLayout->addWidget(categoryLabel);
    filterLayout->addWidget(m_categoryComboBox);
    filterLayout->addStretch();
    
    // Create exercise list section
    auto* exerciseGroupBox = new QGroupBox(tr("Available Exercises"), this);
    auto* exerciseLayout = new QVBoxLayout(exerciseGroupBox);
    
    m_exerciseListWidget = new QListWidget(this);
    exerciseLayout->addWidget(m_exerciseListWidget);
    
    // Create details section
    auto* detailsGroupBox = new QGroupBox(tr("Exercise Details"), this);
    auto* detailsLayout = new QVBoxLayout(detailsGroupBox);
    
    m_exerciseDescriptionTextEdit = new QTextEdit(this);
    m_exerciseDescriptionTextEdit->setReadOnly(true);
    
    auto* progressLabel = new QLabel(tr("Your Progress:"), this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    
    detailsLayout->addWidget(m_exerciseDescriptionTextEdit);
    detailsLayout->addWidget(progressLabel);
    detailsLayout->addWidget(m_progressBar);
    
    // Create button section
    auto* buttonLayout = new QHBoxLayout();
    
    m_createCustomButton = new QPushButton(tr("Create Custom Exercise"), this);
    m_selectButton = new QPushButton(tr("Select"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    
    m_selectButton->setEnabled(false);
    
    buttonLayout->addWidget(m_createCustomButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_selectButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // Add all sections to main layout
    mainLayout->addWidget(filterGroupBox);
    
    // Create a horizontal layout for the list and details
    auto* contentLayout = new QHBoxLayout();
    contentLayout->addWidget(exerciseGroupBox, 1);
    contentLayout->addWidget(detailsGroupBox, 1);
    mainLayout->addLayout(contentLayout, 1);
    
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

void ExerciseBrowser::setupConnections()
{
    connect(m_difficultyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExerciseBrowser::onDifficultyChanged);
            
    connect(m_categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExerciseBrowser::onCategoryChanged);
            
    connect(m_exerciseListWidget, &QListWidget::itemClicked,
            this, &ExerciseBrowser::onExerciseSelected);
            
    connect(m_createCustomButton, &QPushButton::clicked,
            this, &ExerciseBrowser::onCreateCustomExerciseClicked);
            
    connect(m_selectButton, &QPushButton::clicked,
            this, &ExerciseBrowser::onSelectExerciseClicked);
            
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

void ExerciseBrowser::populateExercises()
{
    // Clear existing items
    m_exerciseListWidget->clear();
    
    if (!m_repository) {
        // If no repository is set, create a default in-memory repository
        m_repository = MusicTrainer::adapters::InMemoryExerciseRepository::create();
    }
    
    // Get exercises based on selected filters
    std::vector<std::shared_ptr<domain::exercises::Exercise>> exercises;
    
    // Filter by difficulty
    if (m_difficultyComboBox->currentIndex() == 0) {
        // "All Levels" selected
        exercises = m_repository->getAllExercises();
    } else if (m_difficultyComboBox->currentIndex() == 1) {
        // "Beginner" selected
        exercises = m_repository->getExercisesByDifficulty(domain::exercises::ExerciseDifficulty::BEGINNER);
    } else if (m_difficultyComboBox->currentIndex() == 2) {
        // "Intermediate" selected
        exercises = m_repository->getExercisesByDifficulty(domain::exercises::ExerciseDifficulty::INTERMEDIATE);
    } else if (m_difficultyComboBox->currentIndex() == 3) {
        // "Advanced" selected
        exercises = m_repository->getExercisesByDifficulty(domain::exercises::ExerciseDifficulty::ADVANCED);
    }
    
    // Further filter by category if needed
    if (m_categoryComboBox->currentIndex() > 0) {
        // Category filter is active
        domain::exercises::ExerciseCategory category;
        
        switch (m_categoryComboBox->currentIndex()) {
            case 1: category = domain::exercises::ExerciseCategory::FIRST_SPECIES; break;
            case 2: category = domain::exercises::ExerciseCategory::SECOND_SPECIES; break;
            case 3: category = domain::exercises::ExerciseCategory::THIRD_SPECIES; break;
            case 4: category = domain::exercises::ExerciseCategory::FOURTH_SPECIES; break;
            case 5: category = domain::exercises::ExerciseCategory::FIFTH_SPECIES; break;
            case 6: category = domain::exercises::ExerciseCategory::FREE_COUNTERPOINT; break;
            default: break;
        }
        
        // If we already filtered by difficulty, further filter by category
        if (m_difficultyComboBox->currentIndex() > 0) {
            // Create a temporary vector to store filtered exercises
            std::vector<std::shared_ptr<domain::exercises::Exercise>> filteredExercises;
            
            for (const auto& exercise : exercises) {
                if (exercise->getCategory() == category) {
                    filteredExercises.push_back(exercise);
                }
            }
            
            exercises = std::move(filteredExercises);
        } else {
            // If we didn't filter by difficulty, get exercises by category directly
            exercises = m_repository->getExercisesByCategory(category);
        }
    }
    
    // Add exercises to the list widget
    for (const auto& exercise : exercises) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(exercise->getName()));
        item->setData(Qt::UserRole, QVariant::fromValue(QString::fromStdString(exercise->getName())));
        m_exerciseListWidget->addItem(item);
    }
    
    // Clear selection and details
    m_exerciseDescriptionTextEdit->clear();
    m_progressBar->setValue(0);
    m_selectButton->setEnabled(false);
    m_selectedExercise = nullptr;
}

void ExerciseBrowser::updateExerciseDetails()
{
    auto* selectedItem = m_exerciseListWidget->currentItem();
    if (!selectedItem || !m_repository) {
        m_exerciseDescriptionTextEdit->clear();
        m_progressBar->setValue(0);
        m_selectButton->setEnabled(false);
        return;
    }
    
    // Get the exercise name from the item data
    QString exerciseName = selectedItem->data(Qt::UserRole).toString();
    
    // Get the exercise from the repository
    auto exerciseOpt = m_repository->getExerciseByName(exerciseName.toStdString());
    if (!exerciseOpt.has_value() || !exerciseOpt.value()) {
        m_exerciseDescriptionTextEdit->clear();
        m_progressBar->setValue(0);
        m_selectButton->setEnabled(false);
        return;
    }
    
    // Store the selected exercise
    m_selectedExercise = exerciseOpt.value();
    
    // Update the description
    m_exerciseDescriptionTextEdit->setText(QString::fromStdString(m_selectedExercise->getDescription()));
    
    // Set a random progress value for now
    // In a real implementation, this would come from user data
    int progress = 0;
    switch (m_selectedExercise->getDifficulty()) {
        case domain::exercises::ExerciseDifficulty::BEGINNER:
            progress = 75;
            break;
        case domain::exercises::ExerciseDifficulty::INTERMEDIATE:
            progress = 50;
            break;
        case domain::exercises::ExerciseDifficulty::ADVANCED:
            progress = 25;
            break;
    }
    
    m_progressBar->setValue(progress);
    m_selectButton->setEnabled(true);
}

void ExerciseBrowser::onDifficultyChanged(int index)
{
    Q_UNUSED(index);
    populateExercises();
}

void ExerciseBrowser::onCategoryChanged(int index)
{
    Q_UNUSED(index);
    populateExercises();
}

void ExerciseBrowser::onExerciseSelected(QListWidgetItem* item)
{
    Q_UNUSED(item);
    updateExerciseDetails();
}

void ExerciseBrowser::onCreateCustomExerciseClicked()
{
    QMessageBox::information(this, tr("Create Custom Exercise"),
                            tr("Custom exercise creation will be implemented in a future update."));
}

void ExerciseBrowser::onSelectExerciseClicked()
{
    if (!m_selectedExercise) {
        return;
    }
    
    // Emit the signal with the selected exercise
    Q_EMIT exerciseSelected(m_selectedExercise);
    
    // Show a confirmation message
    QMessageBox::information(this, tr("Exercise Selected"),
                            tr("Selected exercise: %1").arg(QString::fromStdString(m_selectedExercise->getName())));
    
    accept();
}

} // namespace presentation
} // namespace MusicTrainer