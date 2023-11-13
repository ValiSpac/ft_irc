/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:26:02 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/13 15:03:50 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        Server server("6667", "your_password"); // Remplacer par le port et le mot de passe souhaités
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

