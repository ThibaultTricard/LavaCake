#!/bin/bash

# Script to build Doxygen documentation for LavaCake

echo "Building LavaCake Documentation..."
echo "=================================="

# Check if doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed."
    echo "Install it with: sudo apt-get install doxygen graphviz"
    exit 1
fi

# Check if graphviz (dot) is installed for diagrams
if ! command -v dot &> /dev/null; then
    echo "Warning: Graphviz (dot) is not installed. Diagrams will be disabled."
    echo "Install it with: sudo apt-get install graphviz"
fi

# Clean previous documentation
if [ -d "documentation/html" ]; then
    echo "Cleaning previous documentation..."
    rm -rf documentation/html
fi

# Run doxygen from documentation directory
echo "Running Doxygen..."
cd documentation
doxygen Doxyfile
cd ..

# Check if documentation was generated successfully
if [ -d "documentation/html" ]; then
    echo ""
    echo "Documentation built successfully!"
    echo "=================================="
    echo "Output directory: documentation/html"
    echo "Open: documentation/html/index.html"
    echo ""

    # Ask if user wants to open the documentation
    read -p "Would you like to open the documentation now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        # Try to open with default browser
        if command -v xdg-open &> /dev/null; then
            xdg-open documentation/html/index.html
        elif command -v gnome-open &> /dev/null; then
            gnome-open documentation/html/index.html
        elif command -v kde-open &> /dev/null; then
            kde-open documentation/html/index.html
        else
            echo "Could not detect browser opener. Please open documentation/html/index.html manually."
        fi
    fi
else
    echo "Error: Documentation generation failed."
    exit 1
fi
