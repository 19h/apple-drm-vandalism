#pragma once

#include "apple_music_bridge.h"
#include "context.h"
#include <stdbool.h>

/**
 * @brief Initiates and manages the user authentication flow.
 *
 * This function starts the authentication process using the RequestContext from
 * the provided app_context. It relies on the dialog and credential handlers

 * being registered beforehand.
 *
 * @param context The application context, containing the RequestContext and
 *                user credentials from the config.
 * @return true if authentication is successful, false otherwise.
 */
bool auth_perform_login(app_context_t *context);

/**
 * @brief Callback handler for protocol dialogs presented by the auth flow.
 *
 * This function is called by the Apple Music library to handle UI dialogs.
 * It programmatically responds to prompts like "Sign In" by selecting the
 * appropriate button (e.g., "Use Existing Apple ID").
 *
 * @param j A handle or identifier for the dialog.
 * @param proto_dialog_ptr A shared_ptr to the ProtocolDialog object.
 * @param resp_handler A shared_ptr to the response handler.
 */
void auth_dialog_handler(long j, am_shared_ptr_t *proto_dialog_ptr, am_shared_ptr_t *resp_handler);

/**
 * @brief Callback handler for credential requests from the auth flow.
 *
 * This function is called by the Apple Music library when it needs credentials.
 * It provides the username and password from the application config. If a 2FA
 * code is required, it handles reading the code from stdin or a file,
 * appending it to the password, and submitting the response.
 *
 * @param cred_req_handler A shared_ptr to the CredentialsRequest object.
 * @param cred_resp_handler A shared_ptr to the response handler.
 */
void auth_credential_handler(am_shared_ptr_t *cred_req_handler, am_shared_ptr_t *cred_resp_handler);
