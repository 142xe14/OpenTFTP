//
// Created by Julien Debroize on 02/09/2016.
//

#ifndef ADRESSEINTERNET_H
#define ADRESSEINTERNET_H


/**La fonction ci-dessous construit une adresse internet à partir d'une éventuelle adresse(sous forme DNS ou IP) et d'un
 * numéro de port
 */

AdresseInternet * AdresseInternet_new(const char* adresse, uint16_t port);

/**La fonction AdresseInternet_any construit une adresse correspondant à toute les interfaces réseau à partir d'un
 * numéro de port
 */

AdresseInternet * AdresseInternet_any(uint16_t port);

/**La fonction AdresseInternet_loopback construit une adresse internet correspondant à l'interface loopback à partir
 * d'un numéro de port passé en paramètre
 */

AdresseInternet * AdresseInternet_loopback(uint16_t port);
#endif //OPENTFTP_ADRESSEINTERNET_H
