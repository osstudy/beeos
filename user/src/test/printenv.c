/*
 * Copyright (c) 2015-2017, Davide Galassi. All rights reserved.
 *
 * This file is part of the BeeOS software.
 *
 * BeeOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BeeOS; if not, see <http://www.gnu/licenses/>.
 */

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int i;
    char buf[256];
    char *value;

    i = 0;
    while (environ[i] != NULL)
        printf("%s\n", environ[i++]);

    while (1)
    {
        printf("Write an environment variable ('q' to exit)\n> ");
        if (fgets(buf, 256, stdin))
        {
            if (strcmp(buf, "q") == 0)
                break;
            value = getenv(buf);
            if (value)
                printf("%s value is %s\n", buf, value);
            else
                printf("Undefined\n");
        }
    }
    
    return 0;
}
