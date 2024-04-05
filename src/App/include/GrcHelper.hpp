#pragma once

#include "BaseGrc.hpp"
#include "IStorage.hpp"

/*!
* \brief Inference helper funciton.
* \param p_grc Pointer to Grc application.
* \param signal Signal stored in matrix.
* \return Category.
*/
int grc_helper_infer(BaseGrc *p_grc, Matrix &signal);
/*!
* \brief Train helper funciton.
* \param p_grc Pointer to Grc application.
* \param signal Signal stored in matrix.
* \return Category.
*/
int grc_helper_train(BaseGrc *p_grc, Matrix &signal, int train_category);
/*!
* \brief Load weights to Grc helper funciton.
* \param p_grc Pointer to Grc application.
* \param stor Pointer to storage.
* \return Number of loaded categories.
*/
unsigned grc_helper_load_wgts(BaseGrc *p_grc, IStorage *stor);
/*!
* \brief Save weights from Grc helper funciton.
* \param p_grc Pointer to Grc application.
* \param stor Pointer to storage.
* \return Number of saved categories.
*/
unsigned grc_helper_save_wgts(BaseGrc *p_grc, IStorage *stor);
/*!
* \brief Clear weights and Grc state helper funciton.
* \param p_grc Pointer to Grc application.
* \param stor Pointer to storage.
*/
void grc_helper_clear_wgts(BaseGrc *p_grc, IStorage *stor);