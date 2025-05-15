// utilities.js

function showSuccessMessage(message) {
    Swal.fire({
        icon: 'success', title: 'Succes', text: message, timer: 2000, showConfirmButton: false
    });
}

function showErrorMessage(message) {
    Swal.fire({
        icon: 'error', title: 'Eroare', text: message
    });
}

function validateNumberInput(inputElement, fieldName, min = -Infinity, max = Infinity) {
    const value = parseFloat(inputElement.value);
    if (isNaN(value) || value < min || value > max) {
        showErrorMessage(`Vă rugăm să introduceți o valoare validă pentru ${fieldName}.`);
        inputElement.focus();
        return false;
    }
    return true;
}
