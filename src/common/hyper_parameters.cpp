#include "custom_types.hpp"

HP g_HP_Ventilator = {
    false,       // m_PredictSignal
    true,        // m_SeparateInaccuracies
    3,           // m_InputComponents
    3,           // m_OutputComponents
    19,          // m_ReservoirNeurons
    0.45813,     // m_SpectralRadius
    0.255005,    // m_Sparsity
    0.00111267,  // m_Noise
    0.357373,    // m_InputScaling
    0.507581,    // m_InputSparsity
    0.000584534, // m_FeedbackScaling
    0.773157,    // m_FeedbackSparsity
    0.969788     // m_ThresholdFactor
};

HP g_HP_Gesture = {
    false,       // m_PredictSignal
    false,       // m_SeparateInaccuracies
    6,           // m_InputComponents
    6,           // m_OutputComponents
    17,          // m_ReservoirNeurons
    0.372852,    // m_SpectralRadius
    0.459827,    // m_Sparsity
    0.00022583,  // m_Noise
    0.36936,     // m_InputScaling
    0.498682,    // m_InputSparsity
    0.000177185, // m_FeedbackScaling
    0.727765,    // m_FeedbackSparsity
    1.05014      // m_ThresholdFactor
};

HP g_HP_Knock = {
    true,       // m_PredictSignal
    false,      // m_SeparateInaccuracies
    1,          // m_InputComponents
    1,          // m_OutputComponents
    18,         // m_ReservoirNeurons
    0.444104,   // m_SpectralRadius
    0.631049,   // m_Sparsity
    0.00100195, // m_Noise
    0.566296,   // m_InputScaling
    0.4302,     // m_InputSparsity
    0.00168945, // m_FeedbackScaling
    0.417676,   // m_FeedbackSparsity
    0.86792     // m_ThresholdFactor
};
